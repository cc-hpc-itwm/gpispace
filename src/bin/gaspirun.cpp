#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_file.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/signal_handler_manager.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include <csignal>

namespace
{
  struct bg_job
  {
    bg_job (const std::string&host, const std::size_t proc, std::future<int> &&f)
      : host (host)
      , proc (proc)
      , future (std::move(f))
    {}

    std::string host;
    std::size_t proc;
    std::future<int> future;
  };

  namespace option
  {
    struct _
    {
      _ (const std::string& _long, boost::optional<const char*> _short = boost::none)
        : name (_long)
        , descriptor
          ( _short
          ? (name + "," + *_short)
          : name
          )
      {}

      const std::string name;
      const std::string descriptor;

      operator const char*() const
      {
        return descriptor.c_str();
      }
    };

    static const _ help ("help", "h");
    static const _ command ("command", "x");
    static const _ machinefile ("machinefile", "m");
    static const _ numa ("numa", "N");
    static const _ nproc ("nproc", "n");
    static const _ verbose ("verbose", "v");
  }

  struct host_with_multiplicity
  {
    host_with_multiplicity (std::string const & host, std::size_t multiplicity)
      : host (host)
      , multiplicity (multiplicity)
    {}

    std::string host;
    std::size_t multiplicity;
  };

  typedef std::vector<host_with_multiplicity> machinefile_t;

  machinefile_t
  read_machinefile (const boost::filesystem::path& mfile)
  {
    machinefile_t machinefile;

    std::ifstream ifs (mfile.c_str());

    boost::optional<std::string> previous;
    while (ifs)
    {
      std::string line;
      std::getline (ifs, line);

      if (line.empty() || line.front() == '#')
      {
        continue;
      }

      if (previous == line)
      {
        ++machinefile.back().multiplicity;
      }
      else
      {
        machinefile.push_back (host_with_multiplicity (line, 1));
        previous = line;
      }
    }

    return machinefile;
  }

  std::future<int> rexec ( const std::string& host
                         , const std::map<std::string, std::string>& environment
                         , const std::vector<std::string>& command
                         , bool verbose
                         )
  {
    return std::async (std::launch::async, [=,&host] {
        std::ostringstream cmd;

        cmd << "ssh " << host << " /usr/bin/env";
        for (auto kv : environment)
        {
          cmd << " " + kv.first + "=\"" + kv.second + "\"";
        }

        for (auto arg : command)
        {
          cmd << " \"" << arg << "\"";
        }

        if (verbose)
        {
          std::cout << "rexec(" << cmd.str() << ")" << std::endl;
        }

        return system (cmd.str().c_str());
      }
    );
  }

  void kill_and_wait_for_children (std::list<bg_job>& jobs)
  {
    ::kill (0, SIGTERM);

    for (bg_job& job : jobs)
    {
      job.future.get();
    }
  }
}

int main (int argc, char *argv[])
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description visible_options ("Allowed options");
  boost::program_options::options_description cmdline_options;
  boost::program_options::options_description generic_options;
  boost::program_options::options_description advanced_options ("Advanced options");
  boost::program_options::options_description hidden_options;

  generic_options.add_options()
    (option::help, "print usage information")
    ( option::machinefile
    , boost::program_options::value<validators::nonempty_file>()->required()
    , "machinefile to use, this file has to reside in a shared folder, we currently do not transfer it"
    )
    (option::verbose, "be more verbose");

  advanced_options.add_options()
    (option::numa, "enable NUMA support for processes on the same node")
    ( option::nproc
    , boost::program_options::value<validators::positive_integral<unsigned long>>()
    , "start at most #nproc processes from machinefile (unsupported)"
    );

  hidden_options.add_options()
    ( option::command
    , boost::program_options::value<std::vector<std::string>> ()->multitoken()
    , "command to execute"
    );

  visible_options.add (generic_options).add (advanced_options);
  cmdline_options.add (visible_options).add (hidden_options);

  boost::program_options::positional_options_description po;
  po.add (option::command.name.c_str(), -1);

  boost::program_options::variables_map vm;
  boost::program_options::parsed_options parsed
    ( boost::program_options::command_line_parser (argc, argv)
    . options (cmdline_options).positional (po).allow_unregistered()
    . run ()
    );
  boost::program_options::store (parsed, vm);

  if (vm.count (option::help.name))
  {
    std::cout << "usage: " << argv[0] << " -m machinefile [options] [--] binary args..." << std::endl
              << std::endl
              << "hint: use '--' in case args contains allowed parameters to gaspirun"
              << std::endl
              << std::endl
              << visible_options;
    return EXIT_SUCCESS;
  }

  vm.notify();

  boost::filesystem::path const mfile
    (vm [option::machinefile.name].as<validators::nonempty_file>());

  const bool verbose (vm.count (option::verbose.name) > 0);

  if (verbose)
  {
    std::cout << "Using machinefile: " << mfile << std::endl;
  }

  std::vector<std::string> command
    (boost::program_options::collect_unrecognized (parsed.options, boost::program_options::include_positional));

  if (command.empty())
  {
    throw std::runtime_error ("nothing to execute, please specify a valid command");
  }

  boost::filesystem::path const executable
    (validators::executable (command.front()));
  command.front() = boost::filesystem::canonical (executable).string();

  // parse machine file
  machinefile_t machinefile (read_machinefile (mfile));

  if (machinefile.empty())
  {
    throw std::runtime_error ("at least one node is required in machinefile");
  }

  std::list<bg_job> jobs;

  fhg::util::signal_handler_manager signal_handlers;
  signal_handlers.add (SIGTERM, std::bind (kill_and_wait_for_children, std::ref (jobs)));
  signal_handlers.add (SIGINT, std::bind (kill_and_wait_for_children, std::ref (jobs)));

  boost::optional<std::string> master_node;
  bool skip_master (true);

  for (host_with_multiplicity h : machinefile)
  {
    for (std::size_t proc (0); proc < h.multiplicity; ++proc)
    {
      if (skip_master)
      {
        master_node = h.host;
        skip_master = false;
        continue;
      }

      std::map<std::string, std::string> env;
      env["GASPI_MFILE"] = mfile.string();
      env["GASPI_MASTER"] = *master_node;
      env["GASPI_SOCKET"] = std::to_string (proc);
      env["GASPI_TYPE"] = "GASPI_WORKER";
      env["GASPI_SET_NUMA_SOCKET"] = vm.count (option::numa.name) ? "1" : "0";

      if (verbose)
      {
        std::cout << h.host << ": " << proc << "/" << h.multiplicity << std::endl;
      }

      jobs.emplace_back (bg_job (h.host, proc, rexec (h.host, env, command, verbose)));
    }
  }

  {
    const host_with_multiplicity& master (machinefile.front());

    std::map<std::string, std::string> env;
    env["GASPI_MFILE"] = mfile.string();
    env["GASPI_MASTER"] = master.host;
    env["GASPI_SOCKET"] = std::to_string (0);
    env["GASPI_TYPE"] = "GASPI_MASTER";
    env["GASPI_SET_NUMA_SOCKET"] = vm.count (option::numa.name) ? "1" : "0";

    if (verbose)
    {
      std::cout << master.host << ": " << 0 << "/" << master.multiplicity << std::endl;
    }

    jobs.emplace_back (bg_job (master.host, 0, rexec (master.host, env, command, verbose)));
  }

  for (bg_job& job : jobs)
  {
    job.future.get();
  }

  return 0;
}
