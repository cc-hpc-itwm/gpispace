#include <boost/test/included/unit_test.hpp>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/exception.hpp>
#include <fhg/plugin/core/kernel.hpp>

#include <vector>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace fs = boost::filesystem;
using namespace boost::unit_test;

typedef boost::shared_ptr <fhg::core::kernel_t> kernel_ptr_t;
typedef std::vector<fs::path> path_list_t;

static fs::path good_plugin_path;
static fs::path bad_plugin_path;

void test_load_good_plugin ()
{
  kernel_ptr_t krnl(new fhg::core::kernel_t);

  boost::thread krnl_thread
    (boost::bind (&fhg::core::kernel_t::run, krnl));

  krnl->load_plugin (good_plugin_path.string());

  krnl->stop();
  krnl_thread.join();
  krnl->unload_all();
}

void test_load_bad_plugin ()
{
  kernel_ptr_t krnl(new fhg::core::kernel_t);

  boost::thread krnl_thread
    (boost::bind (&fhg::core::kernel_t::run, krnl));

  try
  {
    krnl->load_plugin (bad_plugin_path.string());
  }
  catch (fhg::core::exception::plugin_version_magic_mismatch const &)
  {
    // ok
  }

  krnl->stop();
  krnl_thread.join();
  krnl->unload_all();
}

void test_load_good_and_bad_plugin ()
{
  kernel_ptr_t krnl(new fhg::core::kernel_t);

  boost::thread krnl_thread
    (boost::bind (&fhg::core::kernel_t::run, krnl));

  krnl->load_plugin (good_plugin_path.string());
  try
  {
    krnl->load_plugin (bad_plugin_path.string());
  }
  catch (fhg::core::exception::plugin_version_magic_mismatch const &)
  {
    // ok
  }

  krnl->stop();
  krnl_thread.join();
  krnl->unload_all();
}

test_suite*
init_unit_test_suite (int ac, char *av[])
{
  if (ac <= 1)
  {
    throw boost::unit_test::framework::setup_error
      ("I need the path to the directory where {test-good-magic,test-bad-magic}.so plugins are");
  }

  fs::path prefix = fs::path(av[1]);
  good_plugin_path = prefix / "test-good-magic.so";
  bad_plugin_path  = prefix / "test-bad-magic.so";

  {
    test_suite* ts1 = BOOST_TEST_SUITE( "plugin_kernel" );
    ts1->add( BOOST_TEST_CASE(&test_load_good_plugin));
    ts1->add( BOOST_TEST_CASE(&test_load_bad_plugin));
    ts1->add( BOOST_TEST_CASE(&test_load_good_and_bad_plugin));
    framework::master_test_suite().add( ts1 );
  }

  return 0;
}
