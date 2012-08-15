#include <boost/test/included/unit_test.hpp>

#include <fhg/plugin/plugin.hpp>
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

static fs::path hello_plugin;
static fs::path world_plugin;
static fs::path helloworld_plugin;

void test_start_stop_kernel ()
{
  kernel_ptr_t krnl(new fhg::core::kernel_t);

  boost::thread krnl_thread
    (boost::bind (&fhg::core::kernel_t::run, krnl));

  // load some plugins
  krnl->load_plugin (hello_plugin.string());
  BOOST_REQUIRE (krnl->is_plugin_loaded ("hello"));

  krnl->load_plugin (world_plugin.string());
  BOOST_REQUIRE (krnl->is_plugin_loaded ("world"));

  krnl->load_plugin (helloworld_plugin.string());
  BOOST_REQUIRE (krnl->is_plugin_loaded ("helloworld"));

  boost::this_thread::sleep (boost::posix_time::seconds(2));

  krnl->stop();
  BOOST_REQUIRE (not krnl->is_plugin_loaded ("hello"));
  BOOST_REQUIRE (not krnl->is_plugin_loaded ("world"));
  BOOST_REQUIRE (not krnl->is_plugin_loaded ("helloworld"));

  krnl_thread.join();
  krnl->unload_all();
}

void test_restart_kernel ()
{
  kernel_ptr_t krnl(new fhg::core::kernel_t);

  {
    boost::thread krnl_thread
      (boost::bind (&fhg::core::kernel_t::run, krnl));

    // load some plugins
    krnl->load_plugin (hello_plugin.string());
    krnl->load_plugin (world_plugin.string());
    krnl->load_plugin (helloworld_plugin.string());

    boost::this_thread::sleep (boost::posix_time::seconds(2));

    krnl->stop();
    krnl_thread.join();
    krnl->unload_all();
  }

  {
    boost::thread krnl_thread
      (boost::bind (&fhg::core::kernel_t::run, krnl));

    // load some plugins
    krnl->load_plugin (hello_plugin.string());
    krnl->load_plugin (world_plugin.string());
    krnl->load_plugin (helloworld_plugin.string());

    boost::this_thread::sleep (boost::posix_time::seconds(2));

    krnl->stop();
    krnl_thread.join();
    krnl->unload_all();
  }
}

test_suite*
init_unit_test_suite (int ac, char *av[])
{
  if (ac <= 1)
  {
    throw boost::unit_test::framework::setup_error
      ("I need the path to the directory where {hello,world,helloworld}.so plugins are");
  }

  fs::path prefix = fs::path(av[1]);
  hello_plugin = prefix / "hello.so";
  world_plugin = prefix / "world.so";
  helloworld_plugin = prefix / "helloworld.so";

  {
    test_suite* ts1 = BOOST_TEST_SUITE( "plugin_kernel" );
    ts1->add( BOOST_TEST_CASE(&test_start_stop_kernel));
    ts1->add( BOOST_TEST_CASE(&test_restart_kernel));

    framework::master_test_suite().add( ts1 );
  }

  return 0;
}
