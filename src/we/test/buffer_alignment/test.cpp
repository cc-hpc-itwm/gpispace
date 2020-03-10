#include <boost/test/unit_test.hpp>

#include <net_with_arbitrary_buffer_sizes_and_default_alignments.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/virtual_memory.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>
#include <test/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

BOOST_AUTO_TEST_CASE (arbitrary_buffer_sizes_and_default_alignments)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::options::virtual_memory());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / "buffer_alignment"
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);
  test::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  unsigned long total_buffer_size (0);

  fhg::util::temporary_path const _workflow_dir
   (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const workflow_dir (_workflow_dir);

  boost::filesystem::ofstream ofs
    ( workflow_dir
    /"net_with_arbitrary_buffer_sizes_and_default_alignments.xpnet"
    );

  std::string net_description
    (net_with_arbitrary_buffer_sizes_and_default_alignments
       (total_buffer_size)
    );
  ofs << net_description;
  ofs.close();

  test::make_net_lib_install const make
     ( installation
     , "net_with_arbitrary_buffer_sizes_and_default_alignments"
     , workflow_dir
     , installation_dir
     );

  gspc::scoped_runtime_system const drts
     ( vm
     , installation
     , "worker:1," + std::to_string (total_buffer_size)
     , rifds.entry_points()
     );

  std::multimap<std::string, pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
  ( result = gspc::client (drts).put_and_run
      (gspc::workflow (make.pnet()), {{"start", we::type::literal::control()}})
  );

  BOOST_REQUIRE_EQUAL (result.size(), 1);

  BOOST_REQUIRE_EQUAL (result.count ("done"), 1);
  BOOST_CHECK_EQUAL
    ( result.find ("done")->second
    , pnet::type::value::value_type (we::type::literal::control())
    );
}
