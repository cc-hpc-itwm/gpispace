#include <boost/test/unit_test.hpp>

#include <iml/client/iml.hpp>
#include <iml/client/scoped_rifd.hpp>
#include <iml/client/virtual_memory.hpp>
#include <iml/client/scoped_allocation.hpp>

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/optional/optional_io.hpp>

#include <iml/testing/parse_command_line.hpp>
#include <iml/testing/scoped_nodefile_from_environment.hpp>
#include <iml/testing/shared_directory.hpp>
#include <iml/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <drts/drts_iml.hpp>

#include <we/type/literal/control.hpp>
#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/split.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <regex>
#include <sstream>
#include <vector>

BOOST_AUTO_TEST_CASE (iml_gaspi_vmem_allocation)
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
    (iml_test::shared_directory (vm) / "iml_gaspi_vmem_allocation");

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

  iml_client::scoped_iml_runtime_system const iml_rts
    ( vm
    , installation
    , scoped_rifds.entry_points()
    , std::cerr
    );

  std::size_t num_bytes = 1024*1024;
  iml_client::vmem_allocation const allocation_data
    ( iml_rts.alloc ( iml_client::vmem::gaspi_segment_description()
                    , num_bytes
                    , "data"
                    )
    );

  auto const mem_range
    (gspc::pnet::vmem::range_to_value (allocation_data.global_memory_range()));

  pnet::type::value::structured_type range
    (boost::get<pnet::type::value::structured_type> (mem_range));
  std::ostringstream oss;
  oss << pnet::type::value::show (range);

  std::regex const range_regex
    ( std::string ("Struct \\[handle := Struct \\[name := \"0x[0-9]+\"\\],")
    + " offset := 0UL, size := [1-9][0-9]+UL\\]"
    );

  BOOST_REQUIRE
    ( std::regex_match
      ( oss.str()
      , range_regex
      )
    );
}
