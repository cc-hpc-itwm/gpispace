// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE drts_parallel_running_workflows
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <future>
#include <map>

BOOST_AUTO_TEST_CASE (drts_parallel_running_workflows)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

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
    (test::shared_directory (vm) / "drts_parallel_running_workflows");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  fhg::util::temporary_file const temporary_file_a
    (shared_directory / boost::filesystem::unique_path());
  fhg::util::temporary_file const temporary_file_b
    (shared_directory / boost::filesystem::unique_path());

  boost::filesystem::path const filename_a (temporary_file_a);
  boost::filesystem::path const filename_b (temporary_file_b);

  test::make const make_wait_then_touch
    ( installation
    , "wait_then_touch"
    , test::source_directory (vm)
    , {{"LIB_DESTDIR", installation_dir.string()}}
    , "net lib install"
    );
  test::make const make_touch_then_wait
    ( installation
    , "touch_then_wait"
    , test::source_directory (vm)
    , {{"LIB_DESTDIR", installation_dir.string()}}
    , "net lib install"
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:2", rifds.entry_points());

  auto submit_fun
    ( [&filename_a, &filename_b, &drts]
      (std::string pnet, std::string port, test::make const& make)
    {
      std::multimap<std::string, pnet::type::value::value_type> const result
        ( gspc::client (drts).put_and_run
          ( gspc::workflow (make.build_directory() / pnet)
          , { {"filename_a", filename_a.string()}
            , {"filename_b", filename_b.string()}
            , {"timeout_in_seconds", 5U}
            }
          )
        );

      return result.count (port) == 1
        && result.find (port)->second == pnet::type::value::value_type (true);
    }
    );

  std::future<bool> wait_then_touch
    ( std::async ( std::launch::async
                 , submit_fun
                 , "wait_then_touch.pnet"
                 , "a_existed"
                 , std::cref (make_wait_then_touch)
                 )
    );
  std::future<bool> touch_then_wait
    ( std::async ( std::launch::async
                 , submit_fun
                 , "touch_then_wait.pnet"
                 , "b_existed"
                 , std::cref (make_touch_then_wait)
                 )
    );

  BOOST_CHECK (wait_then_touch.get());
  BOOST_CHECK (touch_then_wait.get());
}
