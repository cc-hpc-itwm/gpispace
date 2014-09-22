#include <fhg/syscall.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <drts/drts.hpp>
#include <drts/vmem.hpp>
#include <drts/rif.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <future>
#include <iostream>
#include <unordered_set>
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

    static const _ help ("help");
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

  std::pair<std::list<std::string>, unsigned long>
  read_nodes (boost::filesystem::path const& nodefile)
  {
    std::unordered_set<std::string> unique_nodes;
    std::list<std::string> nodes;

    {
      std::ifstream stream (nodefile.string());

      std::string node;

      while (std::getline (stream, node))
      {
        unique_nodes.insert (node);
        nodes.emplace_back (node);
      }
    }

    if (unique_nodes.empty())
    {
      throw std::runtime_error
        (( boost::format ("nodefile %1% does not contain nodes")
         % nodefile
         ).str()
        );
    }

    return std::make_pair (nodes, unique_nodes.size());
  }
}

int main (int argc, char *argv[])
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description visible_options ("Allowed options");
  boost::program_options::options_description cmdline_options;
  boost::program_options::options_description generic_options ("Generic options");

  generic_options.add_options()
    (option::help, "print usage information")
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
    .add (gspc::options::drts())
    .add (gspc::options::virtual_memory())
    .add (gspc::options::logging())
    ;
  cmdline_options.add (visible_options);

  boost::program_options::variables_map vm;
  boost::program_options::parsed_options parsed
    ( boost::program_options::command_line_parser (argc, argv)
    . options (cmdline_options).allow_unregistered()
    . run ()
    );
  boost::program_options::store (parsed, vm);

  if (vm.count (option::help.name))
  {
    std::cout << "usage: " << argv[0] << "[options]" << std::endl
              << std::endl
              << visible_options;
    return EXIT_SUCCESS;
  }

  vm.notify();

  fhg::util::signal_handler_manager signal_handlers;

  gspc::rif_t rif ("/dev/shm/rif-" + std::to_string (getuid()));

  gspc::vmem_t vmem ( vm
                    , gspc::installation (vm)
                    , rif
                    , read_nodes (vm["nodefile"].as<validators::existing_path>())
                    , vm[option::kvs_host.name].as<validators::nonempty_string>()
                    , vm[option::kvs_port.name].as<validators::positive_integral<unsigned short>>()
                    );

  boost::optional<pid_t> child (fhg::util::fork_and_daemonize_child());
  if (child)
  {
    std::cout << *child << std::endl;
    exit (EXIT_SUCCESS);
  }

  fhg::util::thread::event<void> done;

  signal_handlers.add (SIGINT, [&done] (int, siginfo_t*, void*) {
      std::async (std::launch::async, [&done] {
          done.notify();
        });
    });
  signal_handlers.add (SIGTERM, [&done] (int, siginfo_t*, void*) {
      std::async (std::launch::async, [&done] {
          done.notify();
        });
    });
  signal_handlers.add (SIGCHLD, [&done] (int, siginfo_t*, void*) {
      std::async (std::launch::async, [&done] {
          done.notify();
        });
    });

  done.wait();

  return 0;
}
