// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_workerlist
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/private/option.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/hostname.hpp>
#include <fhg/util/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <map>
#include <set>
#include <string>

namespace
{
  void run_test ( unsigned long num_worker
                , boost::program_options::variables_map const& vm
                , gspc::installation const& installation
                , test::make const& make
                , std::set<std::string> const& hostnames
                )
  {
    gspc::scoped_runtime_system const drts
      (vm, installation, "worker:" + std::to_string (num_worker));

    std::multimap<std::string, pnet::type::value::value_type> const result
      ( gspc::client (drts)
      . put_and_run
        ( gspc::workflow (make.build_directory() / "workerlist.pnet")
        , {{"num_workers", num_worker}}
        )
      );

    BOOST_REQUIRE_EQUAL (result.size(), 2);

    std::string const port_workers ("workers");
    std::string const port_hostnames ("hostnames");

    BOOST_REQUIRE_EQUAL (result.count (port_workers), 1);
    BOOST_REQUIRE_EQUAL (result.count (port_hostnames), 1);

    //! \todo needs to be a function on the drts topology
    //! or the topology has to answer 'is_worker_valid (worker))'
    std::set<std::string> const worker_names
      ([&hostnames, &num_worker] () -> std::set<std::string>
        {
          std::set<std::string> workers;
          for (std::string const& hostname : hostnames)
          {
            for (unsigned long i (0); i < num_worker; ++i)
            {
              workers.emplace ( ( boost::format ("worker-%1%-%2%")
                                % hostname
                                % (i + 1)
                                ).str()
                              );
            }
          }
          return workers;
        } ()
      );

    for ( std::pair< pnet::type::value::value_type
                   , pnet::type::value::value_type
                   > const& worker_with_host
        : boost::get<std::map< pnet::type::value::value_type
                             , pnet::type::value::value_type
                             >
                    > (result.find (port_hostnames)->second)
        )
    {
      BOOST_REQUIRE (hostnames.find (boost::get<std::string> (worker_with_host.second)) != hostnames.end());
      BOOST_REQUIRE (worker_names.find (boost::get<std::string> (worker_with_host.first)) != worker_names.end());
    }

    std::set<pnet::type::value::value_type> seen_workers;
    for ( pnet::type::value::value_type const& worker
        : boost::get<std::list<pnet::type::value::value_type>
                    > (result.find (port_workers)->second)
        )
    {
      BOOST_REQUIRE (worker_names.find (boost::get<std::string> (worker)) != worker_names.end());
      BOOST_REQUIRE (seen_workers.emplace (worker).second);
    }
  }
}

BOOST_AUTO_TEST_CASE (share_example_workerlist)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());

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
    (test::shared_directory (vm) / "share_example_workerlist");

  test::scoped_state_directory const state_directory (shared_directory, vm);
  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  std::set<std::string> const hostnames
    ([&vm] () -> std::set<std::string>
      {
        std::set<std::string> hosts;
        std::ifstream ifs (gspc::require_nodefile (vm).string());
        while (ifs)
        {
          std::string host;
          std::getline (ifs, host);
          hosts.emplace (host);
        }
        return hosts;
      } ()
    );

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "workerlist"
    , test::source_directory (vm)
    , { {"LIB_DESTDIR", installation_dir.string()}
      , {"CXXLIBRARYPATHS", (installation.gspc_home() / "lib").string()}
      }
    , "net lib install"
    );

  run_test (1, vm, installation, make, hostnames);
  run_test (2, vm, installation, make, hostnames);
  run_test (5, vm, installation, make, hostnames);
}
