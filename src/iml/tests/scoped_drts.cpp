#include <boost/test/unit_test.hpp>

#include <iml/client/iml.hpp>
#include <iml/client/scoped_rifd.hpp>

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/optional/optional_io.hpp>

#include <iml/testing/parse_command_line.hpp>
#include <iml/testing/scoped_nodefile_from_environment.hpp>
#include <iml/testing/shared_directory.hpp>
#include <iml/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/split.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <regex>
#include <sstream>
#include <vector>

BOOST_AUTO_TEST_CASE (scoped_iml_rts_startup)
{
  boost::program_options::options_description options_description;

  options_description.add (iml_client::options::installation());
  options_description.add (iml_client::options::scoped_rifd());
  options_description.add (iml_client::options::virtual_memory());
  options_description.add (iml_test::options::shared_directory());

  boost::program_options::variables_map vm
    ( iml_test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (iml_test::shared_directory (vm) / "scoped_iml_rts");

  iml_test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);
  iml_test::set_iml_vmem_socket_path_for_localhost (vm);

  vm.notify();

  iml_client::installation const installation (vm);

  std::vector<std::string> const hosts
    (fhg::util::read_lines (nodefile_from_environment.path()));

  iml_client::scoped_rifds const scoped_rifds
    ( iml_client::iml_rifd::strategy {vm}
    , iml_client::iml_rifd::hostnames {vm}
    , iml_client::iml_rifd::port {vm}
    , installation
    );

  std::ostringstream info_output_stream;

  {
    iml_client::scoped_iml_runtime_system const iml_rts
      ( vm
      , installation
      , scoped_rifds.entry_points()
      , info_output_stream
      );
  }

  {
    std::vector<std::string> const info_output
      ( fhg::util::split<std::string, std::string, std::vector<std::string>>
          (info_output_stream.str(), '\n')
      );
    BOOST_TEST_CONTEXT (info_output_stream.str())

    BOOST_REQUIRE_EQUAL (info_output.size(), 3);

    std::string const entry_point_master
      ([&info_output, &hosts]()
       {
         std::smatch match;

         BOOST_REQUIRE
           ( std::regex_match
             ( info_output[0]
             , match
             , std::regex
               ("I: starting IML components on ((.+) [0-9]+ [0-9]+)...")
             )
           );
         BOOST_REQUIRE_EQUAL (match.size(), 3);
         BOOST_REQUIRE_EQUAL (match[2].str(), hosts.front());

         return match[1].str();
       }()
      );

    boost::optional<boost::filesystem::path> socket_path
      = iml_client::get_virtual_memory_socket(vm);
    BOOST_REQUIRE (socket_path != boost::none);

    boost::optional<unsigned long> startup_timeout
      = iml_client::get_virtual_memory_startup_timeout(vm);
    BOOST_REQUIRE (startup_timeout != boost::none);

    BOOST_REQUIRE_EQUAL
      ( info_output[1]
      , ( boost::format ("I: starting VMEM on: %1%"
                        " with a timeout of %2%"
                        " seconds"
                        )
        % *socket_path
        % *startup_timeout
        ).str()
      );

    BOOST_REQUIRE
      ( std::regex_match
        ( info_output[2]
        , std::regex { ( boost::format ("terminating vmem on %1%: [0-9]+")
                       % entry_point_master
                       ).str()
                     }
        )
      );
  }
}
