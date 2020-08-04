#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/type/value.hpp>

#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <map>
#include <sstream>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (doc_tutorial_avg_stddev)
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description options_description;

  constexpr char const* const option_name_generator ("generator");

  options_description.add_options()
    ( option_name_generator
    , boost::program_options::value<validators::executable>()->required()
    , "generator program"
    )
    ;

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
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
    (test::shared_directory (vm) / "doc_tutorial_avg_stddev");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  fhg::util::temporary_file _data_file
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const data_file (_data_file);

  test::make_net_lib_install const make
    ( "avg_stddev"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    . add<test::option::gen::include> (test::source_directory (vm) / "include")
    . add<test::option::gen::cxx_flag> ("--std=c++11")
    );

  boost::filesystem::path const generator
    ( boost::filesystem::canonical
      (vm.at (option_name_generator).as<validators::executable>())
    );

  long const num_values (100 << 20);
  long const size_chunk (8 << 20);
  long const size_buffer (8 << 20);
  long const num_buffer (10);

  fhg::util::nest_exceptions<std::runtime_error>
    ([&generator, &data_file]()
     {
       //! \todo inline the generator code instead of calling a binary
       std::ostringstream command_generate;

       command_generate
         << generator
         << " -b " << size_buffer
         << " -n " << num_values
         << " -s 31415926"
         << " -m 0"
         << " -g 1"
         << " -o " << data_file
         ;

       fhg::util::system_with_blocked_SIGCHLD (command_generate.str());
     }
    , "Could not generate data"
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 );
  gspc::scoped_runtime_system const drts
    (vm, "worker:4", rifds.entry_points());

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts)
    . put_and_run ( gspc::workflow (make.pnet())
                  , { {"name_file", data_file.string()}
                    , {"size_file", long (sizeof (long) * num_values)}
                    , {"size_chunk", size_chunk}
                    , {"size_buffer", size_buffer}
                    , {"num_buffer", num_buffer}
                    }
                  )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 2);

  std::string const port_avg ("avg");
  std::string const port_stddev ("stddev");

  BOOST_REQUIRE_EQUAL (result.count (port_avg), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_stddev), 1);

  BOOST_CHECK_SMALL
    (boost::get<double> (result.find (port_avg)->second), 1e-3);
  BOOST_CHECK_CLOSE_FRACTION
    (boost::get<double> (result.find (port_stddev)->second), 1.0, 1e-3);
}
