#include <fhg/syscall.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <drts/drts.hpp>
#include <drts/private/vmem.hpp>
#include <drts/private/rif.hpp>

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
    constexpr const char* help {"help"};
    constexpr const char* kvs_host {"kvs-host"};
    constexpr const char* kvs_port {"kvs-port"};
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
    ( "startup-messages-fifo"
    , boost::program_options::value
        <fhg::util::boost::program_options::existing_path>()->required()
    , "fifo to use for communication during startup (ports used, ...)"
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

  if (vm.count (option::help))
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
                    , vm[option::kvs_host].as<validators::nonempty_string>()
                    , vm[option::kvs_port].as<validators::positive_integral<unsigned short>>()
                    );

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

  {
    std::ofstream startup_messages_fifo
      ( vm["startup-messages-fifo"]
      . as<fhg::util::boost::program_options::existing_path>().string()
      );
    startup_messages_fifo << "OKAY\n";
  }

  done.wait();

  return 0;
}
