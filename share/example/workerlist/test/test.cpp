#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/private/option.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <map>
#include <set>
#include <string>

namespace
{
  void run_test ( unsigned long num_worker
                , boost::program_options::variables_map const& vm
                , gspc::installation const& installation
                , test::make_net_lib_install const& make
                , boost::filesystem::path const& shared_directory
                )
  {
    gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                   , gspc::rifd::hostnames {vm}
                                   , gspc::rifd::port {vm}
                                   , installation
                                   );
    gspc::scoped_runtime_system const drts
      ( vm
      , installation
      , "worker:" + std::to_string (num_worker)
      , rifds.entry_points()
      );

    std::multimap<std::string, pnet::type::value::value_type> const result
      ( gspc::client (drts).put_and_run
          (gspc::workflow (make.pnet()), {{"num_workers", num_worker}})
      );

    BOOST_REQUIRE_EQUAL (result.size(), 2);

    std::string const port_workers ("workers");
    std::string const port_hostnames ("hostnames");

    BOOST_REQUIRE_EQUAL (result.count (port_workers), 1);
    BOOST_REQUIRE_EQUAL (result.count (port_hostnames), 1);

    //! \todo needs to be a function on the drts topology
    //! or the topology has to answer 'is_worker_valid (worker))'
    fhg::util::temporary_file const _entry_points
      (shared_directory / boost::filesystem::unique_path());
    //! \note the only usage of write_to_file
    rifds.entry_points().write_to_file (_entry_points);
    std::vector<std::string> const entry_points
      (fhg::util::read_lines (_entry_points));

    std::set<std::string> const worker_names
      ([&entry_points, &num_worker] () -> std::set<std::string>
        {
          std::set<std::string> workers;
          for (std::string const& entry_point : entry_points)
          {
            for (unsigned long i (0); i < num_worker; ++i)
            {
              workers.emplace ( ( boost::format ("worker-%1%-%2%")
                                % entry_point
                                % (i + 1)
                                ).str()
                              );
            }
          }
          return workers;
        } ()
      );

    //! \note exploits internal knowledge
    std::set<std::string> const hostnames
      ([&entry_points]
       {
         std::set<std::string> hostnames_;

         for (std::string const& entry_point : entry_points)
         {
           hostnames_.emplace (entry_point.substr (0, entry_point.find (' ')));
         }

         return hostnames_;
       }()
      );

    for ( auto const& worker_with_host
        : boost::get<std::map< pnet::type::value::value_type
                             , pnet::type::value::value_type
                             >
                    > (result.find (port_hostnames)->second)
        )
    {
      BOOST_REQUIRE_GT (hostnames.count (boost::get<std::string> (worker_with_host.second)), 0ul);
      BOOST_REQUIRE_GT (worker_names.count (boost::get<std::string> (worker_with_host.first)), 0ul);
    }

    {
      std::set<pnet::type::value::value_type> seen_workers;
      for ( pnet::type::value::value_type const& worker
          : boost::get<std::list<pnet::type::value::value_type>>
              (result.find (port_workers)->second)
          )
      {
        BOOST_REQUIRE_GT (worker_names.count (boost::get<std::string> (worker)), 0ul);
        BOOST_REQUIRE (seen_workers.emplace (worker).second);
      }
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
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "share_example_workerlist");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( "workerlist"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    );

  run_test (1, vm, installation, make, shared_directory);
  run_test (2, vm, installation, make, shared_directory);
  run_test (5, vm, installation, make, shared_directory);
}
