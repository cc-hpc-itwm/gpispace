// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE token_put
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (wait_for_token_put)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
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
    (test::shared_directory (vm) / "wait_for_token_put");

  test::scoped_state_directory const state_directory (shared_directory, vm);
  test::scoped_nodefile_with_localhost const nodefile_with_localhost
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "wait_for_token_put"
    , test::source_directory (vm)
    , {{"LIB_DESTDIR", installation_dir.string()}}
    , "net lib install"
    );

  gspc::scoped_runtime_system const drts (vm, installation, "worker:2");
  gspc::client client (drts);

  gspc::workflow workflow (make.build_directory() / "wait_for_token_put.pnet");

  workflow.set_wait_for_output();

  pnet::type::value::value_type const bad (std::string ("bad"));
  pnet::type::value::value_type const good (std::string ("good"));

  gspc::job_id_t const job_id (client.submit (workflow, {{"in", good}}));

  client.put_token (job_id, "in", bad);

  std::multimap<std::string, pnet::type::value::value_type> const result
    (client.wait_and_extract (job_id));

  std::string const port_bad ("bad");
  std::string const port_good ("good");

  BOOST_REQUIRE_EQUAL (result.count (port_bad), 1);
  BOOST_REQUIRE_EQUAL (result.find (port_bad)->second, bad);
  BOOST_REQUIRE_EQUAL (result.count (port_good), 1);
  BOOST_REQUIRE_EQUAL (result.find (port_good)->second, good);
}