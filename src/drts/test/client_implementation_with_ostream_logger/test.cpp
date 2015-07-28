#define BOOST_TEST_MODULE drts_client_with_ostream_logger
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <drts/private/option.hpp>

#include <fhglog/Logger.hpp>
#include <fhglog/appender/call.hpp>
#include <fhglog/remote/server.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/hostname.hpp>

#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/generic.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <future>

namespace
{
  namespace option
  {
    namespace po = fhg::util::boost::program_options;

    po::option<po::existing_path> const implementation
      {"implementation", "implementation"};
  }
}

BOOST_AUTO_TEST_CASE (client_implementation_with_ostream_logger)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  //! \todo switch to generic interface
  options_description.add_options()
    ( option::implementation.name().c_str()
    , option::implementation()->required()
    , option::implementation.description().c_str()
    );

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      )
    . options (options_description).run()
    , vm
    );

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / "client_implementation_with_ostream_logger"
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  //! \todo get a free port
  unsigned short const log_port (47096);

  gspc::set_log_host (vm, fhg::util::hostname());
  gspc::set_log_port (vm, log_port);

  vm.notify();

  boost::asio::io_service io_service;
  fhg::log::Logger logger;
  fhg::log::remote::LogServer const log_server (logger, io_service, log_port);

  std::promise<std::string> logged;

  logger.addAppender<fhg::log::appender::call>
    ([&logged] (fhg::log::LogEvent const& event)
     {
       logged.set_value (event.message());
     }
    );

  boost::strict_scoped_thread<boost::join_if_joinable> const service_thread
    ([&io_service] { io_service.run(); });

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "client_implementation_with_ostream_logger"
    , test::source_directory (vm)
    , {{"LIB_DESTDIR", installation_dir.string()}
      ,{ "CXXINCLUDEPATHS"
          //! \todo urgh, remove make
       , test::source_directory (vm).parent_path().parent_path().parent_path().string()
       }
      ,{"PNETC_OPTS", std::string ("--gen-cxxflags=--std=c++11")}
      }
    , "net lib install"
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system drts
    (vm, installation, "worker:1", rifds.entry_points());

  boost::filesystem::path const implementation
    (option::implementation.get_from (vm));

  std::string const message
    {fhg::util::testing::random_string_without ("\n\\\"")};

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow
          ( make.build_directory()
          / "client_implementation_with_ostream_logger.pnet"
          )
      , { {"implementation", implementation.string()}
        , {"message", message}
        }
      )
    );

  io_service.stop();

  BOOST_REQUIRE_EQUAL (result.size(), 0);
  BOOST_REQUIRE_EQUAL (logged.get_future().get(), message);
}
