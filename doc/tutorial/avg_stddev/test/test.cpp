// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE doc_tutorial_avg_stddev
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/type/value.hpp>

#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <fhg/util/temporary_file.hpp>
#include <fhg/util/temporary_path.hpp>

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
    (test::shared_directory (vm) / "doc_tutorial_avg_stddev");

  test::scoped_state_directory const state_directory (shared_directory, vm);
  test::scoped_nodefile_with_localhost const nodefile_with_localhost
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  fhg::util::temporary_file _data_file
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const data_file (_data_file);

  test::make const make
    ( installation
    , "avg_stddev"
    , test::source_directory (vm)
    , { {"LIB_DESTDIR", installation_dir.string()}
      , {"DATFILE", data_file.string()}
      , {"CXXINCLUDEPATHS", (test::source_directory (vm) / "include").string()}
      }
    , "net lib install"
    );

  boost::filesystem::path const generator
    ( boost::filesystem::canonical
      (vm[option_name_generator].as<validators::executable>())
    );

  long const num_values (100 << 20);
  long const size_chunk (8 << 20);
  long const size_buffer (8 << 20);
  long const num_buffer (10);

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

  if ( int ec
     = fhg::util::system_with_blocked_SIGCHLD (command_generate.str().c_str())
     )
  {
    throw std::runtime_error
      (( boost::format ("Could not generate data: command '%1%', error '%2%'")
       % command_generate.str()
       % ec
       ).str()
      );
  }

  gspc::scoped_runtime_system const drts (vm, installation, "worker:4");

  std::multimap<std::string, pnet::type::value::value_type> const result
    (drts.put_and_run ( make.build_directory() / "avg_stddev.pnet"
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

  BOOST_CHECK_CLOSE
    (boost::get<double> (result.find (port_avg)->second), 0.000193708, 1e-8);
  BOOST_CHECK_CLOSE
    (boost::get<double> (result.find (port_stddev)->second), 0.999908, 1e-8);
}