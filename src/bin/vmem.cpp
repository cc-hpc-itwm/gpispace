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
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
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
try
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description visible_options ("Allowed options");
  boost::program_options::options_description cmdline_options;
  boost::program_options::options_description generic_options ("Generic options");

  generic_options.add_options()
    (option::help, "print usage information")
    ( "startup-messages-pipe"
    , boost::program_options::value<int>()->required()
    , "pipe filedescriptor to use for communication during startup (ports used, ...)"
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
                    );

  fhg::util::thread::event<void> done;

  auto request_stop ( [&done] (int, siginfo_t*, void*)
                      {
                        std::async ( std::launch::async
                                   , [&done]
                                     {
                                       done.notify();
                                     }
                                   );
                      }
                    );

  fhg::util::scoped_signal_handler const SIGINT_handler
    (signal_handlers, SIGINT, request_stop);
  fhg::util::scoped_signal_handler const SIGTERM_handler
    (signal_handlers, SIGTERM, request_stop);
  fhg::util::scoped_signal_handler const SIGCHLD_handler
    (signal_handlers, SIGCHLD, request_stop);

  {
    boost::iostreams::stream<boost::iostreams::file_descriptor_sink>
      startup_messages_pipe ( vm["startup-messages-pipe"].as<int>()
                            , boost::iostreams::close_handle
                            );
    startup_messages_pipe << "OKAY\n";
  }

  done.wait();

  return 0;
}
catch (std::exception const& ex)
{
  std::cerr << "EX: " << ex.what() << std::endl;
  return 1;
}
