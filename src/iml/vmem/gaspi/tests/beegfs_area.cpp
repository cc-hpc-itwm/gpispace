#include <iml/vmem/gaspi/pc/memory/beegfs_area.hpp>

#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <iml/vmem/gaspi/pc/segment/segment.hpp>
#include <iml/vmem/gaspi/tests/dummy_topology.hpp>

#include <util-generic/syscall.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

#include <iml/testing/shared_directory.hpp>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/test/unit_test.hpp>

#include <fcntl.h>
#include <sys/stat.h>

#include <cstddef>
#include <exception>

struct setup_and_cleanup_shared_file
{
  boost::filesystem::path get_shared_directory()
  {
    boost::program_options::options_description options_description;
    options_description.add (iml_test::options::shared_directory());
    boost::program_options::variables_map vm;
    boost::program_options::store
      ( boost::program_options::command_line_parser
          ( boost::unit_test::framework::master_test_suite().argc
          , boost::unit_test::framework::master_test_suite().argv
          )
      . options (options_description)
      . run()
      , vm
      );
    return iml_test::shared_directory (vm);
  }

  setup_and_cleanup_shared_file()
    : path_to_shared_file (get_shared_directory() / "beegfs_area_test_area")
  {}

  boost::filesystem::path path_to_shared_file;
};

namespace
{
  gpi::pc::type::handle_t random_handle()
  {
    return fhg::util::testing::random<gpi::pc::type::handle_id_t>{}();
  }
}

BOOST_FIXTURE_TEST_CASE (create_beegfs_segment, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;
  const char *text = "hello world!\n";

  {
    beegfs_area_t area ( true
                       , path_to_shared_file
                       , size
                       , topology
                       );

    handle_t handle (random_handle());
    area.alloc (size, "test", gpi::pc::is_global::yes, 2, handle);
    area.write_to (memory_location_t (handle, 0), text, strlen (text));

    {
      int fd = fhg::util::syscall::open
        (((path_to_shared_file / "data").string ().c_str()), O_RDONLY);

      char buf [size];
      std::size_t read_bytes (fhg::util::syscall::read (fd, buf, size));
      BOOST_CHECK_EQUAL (size, gpi::pc::type::size_t (read_bytes));
      fhg::util::syscall::close (fd);

      int eq = strncmp (text, buf, strlen (text));
      BOOST_CHECK_EQUAL (0, eq);
    }

    area.free (handle);
  }
}

BOOST_FIXTURE_TEST_CASE (existing_directory_is_failure, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;

  boost::filesystem::create_directories (path_to_shared_file);

  BOOST_REQUIRE_THROW  ( gpi::pc::memory::beegfs_area_t ( true
                                                        , path_to_shared_file
                                                        , size
                                                        , topology
                                                        )
                       , std::exception
                       );

  boost::filesystem::remove (path_to_shared_file);

  gpi::pc::memory::beegfs_area_t ( true
                                 , path_to_shared_file
                                 , size
                                 , topology
                                 );
}

BOOST_FIXTURE_TEST_CASE (create_big_beegfs_segment, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 35); // 32 GB

  try
  {
    beegfs_area_t area ( true
                       , path_to_shared_file
                       , size
                       , topology
                       );
  }
  catch (std::exception const &ex)
  {
    BOOST_WARN_MESSAGE ( false
                       , "could not allocate beegfs segment of size: " << size
                       << ": " << ex.what ()
                       << " - please check 'ulimit -v'"
                       );
  }
}

BOOST_FIXTURE_TEST_CASE (create_huge_beegfs_segment, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 40); // 1 TB

  try
  {
    beegfs_area_t area ( true
                       , path_to_shared_file
                       , size
                       , topology
                       );
  }
  catch (std::exception const &ex)
  {
    BOOST_ERROR ( "could not allocate beegfs segment of size: " << size
                << ": " << ex.what ()
                << " - please check 'ulimit -v'"
                );
  }
}

BOOST_FIXTURE_TEST_CASE (test_read, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 20); // 1 MB
  const char *text = "hello world!\n";

  try
  {
    beegfs_area_t area ( true
                       , path_to_shared_file
                       , size
                       , topology
                       );

    handle_t handle (random_handle());
    area.alloc (size, "test", gpi::pc::is_global::yes, 2, handle);

    area.write_to ( memory_location_t (handle, 0)
                  , text
                  , strlen (text)
                  );

    char buf [size];
    area.read_from ( memory_location_t (handle, 0)
                   , &buf[0]
                   , size
                   );

    int check = strncmp (text, buf, strlen (text));
    BOOST_CHECK_EQUAL (0, check);

    area.free (handle);
  }
  catch (std::exception const &ex)
  {
    BOOST_ERROR ( "could not allocate beegfs segment of size: " << size
                << ": " << ex.what ()
                << " - please check 'ulimit -v'"
                );
  }
}

BOOST_FIXTURE_TEST_CASE (test_already_open, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 20); // 1 MB

  beegfs_area_t area ( true
                     , path_to_shared_file
                     , size
                     , topology
                     );

  BOOST_REQUIRE_THROW ( beegfs_area_t ( true
                                      , path_to_shared_file
                                      , size
                                      , topology
                                      )
                      , std::exception
                      );
}
