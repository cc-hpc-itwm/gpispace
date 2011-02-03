// mirko.rahn@itwm.fraunhofer.de

#include <string>
#include <list>

#define COMM_TEST 1

#include <comm.hpp>
#include <state.hpp>
#include <splitted_path.hpp>

// ************************************************************************* //

static gpi_fuse::comm::comm comm;

typedef gpi_fuse::state::splitted_path sp_t;

struct test_t
{
  const std::string path;
  const sp_t sp;

  test_t (const std::string & _path, const sp_t & _sp)
    : path (_path)
    , sp (_sp)
  {}
};

typedef std::list<test_t> test_list;

int
main (int argc, char **argv)
{
  comm.init();

  gpi_fuse::state::state state (comm);

  test_list tests;

  tests.push_back (test_t ("/", sp_t ("/")));
  tests.push_back (test_t ("/global", sp_t ("global")));
  tests.push_back (test_t ("/local", sp_t ("local")));
  tests.push_back (test_t ("/shared", sp_t ("shared")));
  tests.push_back (test_t ("/shared/3", sp_t ("shared/3")));
  tests.push_back (test_t ("/shared/9", sp_t ("shared/9")));
  tests.push_back (test_t ("/shared/0", sp_t ("shared/0")));
  tests.push_back (test_t ("/shared/1", sp_t ("shared/1")));
  tests.push_back (test_t ("/shared/7", sp_t ()));
  tests.push_back (test_t ("/global/0", sp_t ("global", 0)));
  tests.push_back (test_t ("/global/4", sp_t()));
  tests.push_back (test_t ("/local/0", sp_t()));
  tests.push_back (test_t ("/local/4", sp_t ("local", 4)));
  tests.push_back (test_t ("/shared/3/301", sp_t ("shared/3", 301)));
  tests.push_back (test_t ("/shared/3/302", sp_t ("shared/3", 302)));
  tests.push_back (test_t ("/shared/3/303", sp_t ()));
  tests.push_back (test_t ("/0", sp_t (0)));
  tests.push_back (test_t ("/1", sp_t (1)));
  tests.push_back (test_t ("/5", sp_t ()));

  tests.push_back (test_t ("/global/data", sp_t()));
  tests.push_back (test_t ("/local/data", sp_t()));
  tests.push_back (test_t ("/shared/data", sp_t()));
  tests.push_back (test_t ("/shared/3/data", sp_t()));
  tests.push_back (test_t ("/shared/9/data", sp_t()));
  tests.push_back (test_t ("/shared/0/data", sp_t()));
  tests.push_back (test_t ("/shared/1/data", sp_t()));
  tests.push_back (test_t ("/shared/7/data", sp_t()));
  tests.push_back (test_t ("/global/0/data", sp_t("global", 0, "data")));
  tests.push_back (test_t ("/global/4/data", sp_t()));
  tests.push_back (test_t ("/local/0/data", sp_t()));
  tests.push_back (test_t ("/local/4/data", sp_t("local", 4, "data")));
  tests.push_back (test_t ("/shared/3/301/data", sp_t("shared/3", 301, "data")));
  tests.push_back (test_t ("/shared/3/302/data", sp_t("shared/3", 302, "data")));
  tests.push_back (test_t ("/shared/3/303/data", sp_t()));
  tests.push_back (test_t ("/0/data", sp_t(0, "data")));
  tests.push_back (test_t ("/1/data", sp_t(1, "data")));
  tests.push_back (test_t ("/5/data", sp_t()));

  for  ( test_list::const_iterator t (tests.begin())
       ; t != tests.end()
       ; ++t
       )
    {
      gpi_fuse::state::splitted_path sp (state.split (t->path));

      std::cout << t->path << ":";
      std::cout << (state.is_file(sp) ? " FILE:" : "");
      std::cout << (state.is_directory(sp) ? " DIRECTORY:" : "");
      std::cout << " " << sp << std::endl;

      if (!(sp == t->sp))
        {
          exit (EXIT_FAILURE);
        }
    }

  std::cout << "SUCCESS" << std::endl;

  return EXIT_SUCCESS;
}
