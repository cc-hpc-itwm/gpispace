#include <fhg/syscall.hpp>
#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_file.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/signal_handler_manager.hpp>

#include <drts/drts.hpp>

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
    static const _ kvs_host ("kvs-host");
    static const _ kvs_port ("kvs-port");
    static const _ log_host ("log-host");
    static const _ log_port ("log-port");
    static const _ log_level ("log-level");
    static const _ socket ("socket");
    static const _ port ("port");
    static const _ memsize ("memory-size");
    static const _ timeout ("timeout");
  }

  typedef std::vector<std::string> machinefile_t;

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

      if (previous != line)
      {
        machinefile.push_back (line);
        previous = line;
      }
    }

    return machinefile;
  }

  namespace validators = fhg::util::boost::program_options;
  class vmem_t
  {
  public:
    vmem_t ( boost::program_options::variables_map const& vm
           , gspc::installation const &installation
           , gspc::rif_t& rif
           )
      : machinefile_path (vm [option::machinefile.name].as<validators::nonempty_file>())
      , machinefile (read_machinefile (machinefile_path))
      , _rif (rif)
    {
      const std::string& kvs_host (vm[option::kvs_host.name].as<validators::nonempty_string>());
      const unsigned short kvs_port (vm[option::kvs_port.name].as<validators::positive_integral<unsigned short>>());
      const std::string& log_host (vm[option::log_host.name].as<validators::nonempty_string>());
      const unsigned short log_port (vm[option::log_port.name].as<validators::positive_integral<unsigned short>>());
      const std::string& log_level (vm[option::log_level.name].as<std::string>());
      const boost::filesystem::path socket (vm[option::socket.name].as<std::string>());
      const unsigned short port (vm[option::port.name].as<validators::positive_integral<unsigned short>>());
      const std::uint64_t memory_size (vm[option::memsize.name].as<validators::positive_integral<std::uint64_t>>());
      const std::uint64_t timeout (vm[option::timeout.name].as<validators::positive_integral<std::uint64_t>>());

      if (machinefile.empty())
      {
        throw std::runtime_error ("at least one node is required in machinefile");
      }

      const validators::executable vmem_binary
        ((installation.gspc_home() / "bin" / "gpi-space").string());

      if (boost::filesystem::exists (socket))
      {
        throw std::runtime_error ("socket file already exists: " + socket.string());
      }

      const std::string& master (machinefile.front());

      const std::list<gspc::rif_t::endpoint_t> rif_endpoints
        ([] (const machinefile_t& m) -> std::list<gspc::rif_t::endpoint_t> {
          std::list<gspc::rif_t::endpoint_t> ep;
          for (std::string const&h : m)
          {
            ep.emplace_back (gspc::rif_t::endpoint_t (h, 22));
          }
          return std::move (ep);
        }(machinefile));

      // create in memory hostlist
      // rif.store (machinefile, hostlist, "<rif-root>/machinefile");
      // rif.exec (machinefile - master, [...]);
      // rif.exec ({master}, [...]);

      {
        std::map<std::string, std::string> env;
        env["GASPI_MFILE"] = machinefile_path.string();
        env["GASPI_MASTER"] = master;
        env["GASPI_SOCKET"] = "0";
        env["GASPI_TYPE"] = "GASPI_MASTER";
        env["GASPI_SET_NUMA_SOCKET"] = "0";

        std::vector<std::string> command
          ({ vmem_binary.string()
              , "--kvs-host", kvs_host, "--kvs-port", std::to_string (kvs_port)
              , "--log-host", log_host, "--log-port", std::to_string (log_port)
              , "--log-level", log_level
              , "--gpi-mem", std::to_string (memory_size)
              , "--socket", socket.string()
              , "--port", std::to_string (port)
              , "--gpi-api", machinefile.size() > 1 ? "gaspi" : "fake"
              , "--gpi-timeout", std::to_string (timeout)
              });

        _rif.exec ({rif_endpoints.front()}
                  , "gaspi-master"
                  , command
                  , env
                  );
      }

      if (machinefile.size () > 1)
      {
        std::map<std::string, std::string> env;
        env["GASPI_MFILE"] = machinefile_path.string();
        env["GASPI_MASTER"] = master;
        env["GASPI_SOCKET"] = "0";
        env["GASPI_TYPE"] = "GASPI_WORKER";
        env["GASPI_SET_NUMA_SOCKET"] = "0";

        std::vector<std::string> command
          ({ vmem_binary.string()
              , "--kvs-host", kvs_host, "--kvs-port", std::to_string (kvs_port)
              , "--log-host", log_host, "--log-port", std::to_string (log_port)
              , "--log-level", log_level
              , "--gpi-mem", std::to_string (memory_size)
              , "--socket", socket.string()
              , "--port", std::to_string (port)
              , "--gpi-api", machinefile.size() > 1 ? "gaspi" : "fake"
              , "--gpi-timeout", std::to_string (timeout)
              });

        _rif.exec ( std::list<gspc::rif_t::endpoint_t> (++rif_endpoints.begin(), rif_endpoints.end())
                  , "gaspi-worker"
                  , command
                  , env
                  );
      }

      std::uint64_t slept (0);
      while (!boost::filesystem::exists (socket))
      {
        const std::uint64_t sleep_duration (1000);

        usleep (sleep_duration * 1000);
        slept += sleep_duration;

        if (slept > timeout)
        {
          throw std::runtime_error
            ( "timeout while waiting for: " + socket.string()
            + " waited: " + std::to_string (slept) + " seconds"
            );
        }
      }
    }

    void stop()
    {
      if (machinefile.size() > 1)
      {
        _rif.stop ({}, "gaspi-worker");
      }

      _rif.stop ({}, "gaspi-master");
    }

    ~vmem_t()
    {
      stop();
    }
  private:
    boost::filesystem::path machinefile_path;
    machinefile_t machinefile;
    gspc::rif_t& _rif;
    boost::mutex status_mutex;
  };
}

int main (int argc, char *argv[])
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description visible_options ("Allowed options");
  boost::program_options::options_description cmdline_options;
  boost::program_options::options_description generic_options ("Generic options");
  boost::program_options::options_description gaspi_options ("GASPI options");
  boost::program_options::options_description hidden_options;

  generic_options.add_options()
    (option::help, "print usage information")
    ( option::machinefile
    , boost::program_options::value<validators::nonempty_file>()->required()
    , "machinefile to use, this file has to reside in a shared folder, we currently do not transfer it"
    )
    ;
  gaspi_options.add_options()
    ( option::socket
    , boost::program_options::value<std::string>()->required()
    , "socket to listen on"
    )
    ( option::timeout
    , boost::program_options::value<validators::positive_integral<std::uint64_t>>()->required()
    , "time to wait until we think that the start-up failed in milliseconds"
    )
    ( option::memsize
    , boost::program_options::value<validators::positive_integral<std::uint64_t>>()->required()
    , "amount of memory to use for GASPI"
    )
    ( option::port
    , boost::program_options::value<validators::positive_integral<unsigned short>>()->required()
    , "port to use for GASPI communication"
    )
    ( option::kvs_host
    , boost::program_options::value<validators::nonempty_string>()->required()
    , "kvs host to use"
    )
    ( option::kvs_port
    , boost::program_options::value<validators::positive_integral<unsigned short>>()->required()
    , "kvs port to use"
    )
    ;

  visible_options
    .add (generic_options)
    .add (gspc::options::installation())
    .add (gaspi_options)
    .add (gspc::options::logging())
    ;
  cmdline_options.add (visible_options).add (hidden_options);

  boost::program_options::variables_map vm;
  boost::program_options::parsed_options parsed
    ( boost::program_options::command_line_parser (argc, argv)
    . options (cmdline_options).allow_unregistered()
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

  fhg::util::signal_handler_manager signal_handlers;

  gspc::rif_t rif ("/dev/shm");

  volatile bool done = false;

  vmem_t vmem (vm, gspc::installation (vm), rif);
  signal_handlers.add (SIGINT, [&done] (int, siginfo_t*, void*) {
      std::async (std::launch::async, [&done] {
          done = true;
        });
    });
  signal_handlers.add (SIGTERM, [&done] (int, siginfo_t*, void*) {
      std::async (std::launch::async, [&done] {
          done = true;
        });
    });

  std::cout << "running with PID " << getpid() << std::endl;

  while (! done)
  {
    usleep (1000);
  }

  std::cerr << "I am done" << std::endl;

  return 0;
}
