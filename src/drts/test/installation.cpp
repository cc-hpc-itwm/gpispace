
#define BOOST_TEST_MODULE drts_installation
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <boost/program_options.hpp>

BOOST_AUTO_TEST_CASE (installation_set_gspc_home)
{
  boost::program_options::variables_map vm;
  gspc::set_gspc_home
    ( vm
    , boost::filesystem::canonical
        (boost::unit_test::framework::master_test_suite().argv[0]).parent_path()
    );
  gspc::installation const installation (vm);
}
