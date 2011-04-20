// mirko.rahn@itwm.fraunhofer.de

#include <string>
#include <list>

#include <gpifs/state.hpp>
#include <gpifs/splitted_path.hpp>

#include <boost/optional.hpp>

// ************************************************************************* //

typedef gpifs::state::splitted_path sp_t;

struct test_t
{
  const std::string path;
  const boost::optional<sp_t> sp;

  test_t (const std::string & _path)
    : path (_path)
    , sp (boost::optional<sp_t> (boost::none))
  {}

  test_t (const std::string & _path, const sp_t & _sp)
    : path (_path)
    , sp (_sp)
  {}
};

typedef std::list<test_t> test_list;

int
main (int argc, char **argv)
{
  gpifs::state::state state;

  state.init();

  test_list tests;

  tests.push_back (test_t ("/", sp_t ("/")));
  tests.push_back (test_t ("/global", sp_t ("global")));
  tests.push_back (test_t ("/local", sp_t ("local")));
  tests.push_back (test_t ("/shared", sp_t ("shared")));
  tests.push_back (test_t ("/shared/4", sp_t ("shared/4")));
  tests.push_back (test_t ("/shared/9", sp_t ("shared/9")));
  tests.push_back (test_t ("/shared/0"));
  tests.push_back (test_t ("/shared/1"));
  tests.push_back (test_t ("/shared/7"));
  tests.push_back (test_t ("/global/0x1006000000000001", sp_t ("global", gpifs::segment::GLOBAL, 0x1006000000000001)));
  tests.push_back (test_t ("/global/4"));
  tests.push_back (test_t ("/local/0"));
  tests.push_back (test_t ("/local/0x2000000000000004", sp_t ("local", gpifs::segment::LOCAL, 0x2000000000000004)));
  tests.push_back (test_t ("/shared/4/0x300000000000012d", sp_t ("shared/4", 4, 0x300000000000012d)));
  tests.push_back (test_t ("/shared/4/0x300000000000012e", sp_t ("shared/4", 4, 0x300000000000012e)));
  tests.push_back (test_t ("/shared/4/303"));
  tests.push_back (test_t ("/0x1006000000000001", sp_t (0x1006000000000001)));
  tests.push_back (test_t ("/0x10a0000000000001", sp_t (0x10a0000000000001)));
  tests.push_back (test_t ("/5"));
  tests.push_back (test_t ("/proc/alloc", sp_t ("proc","alloc")));
  tests.push_back (test_t ("/proc/data"));

  tests.push_back (test_t ("/data"));
  tests.push_back (test_t ("/global/data"));
  tests.push_back (test_t ("/local/data"));
  tests.push_back (test_t ("/shared/data"));
  tests.push_back (test_t ("/shared/4/data"));
  tests.push_back (test_t ("/shared/9/data"));
  tests.push_back (test_t ("/shared/0/data"));
  tests.push_back (test_t ("/shared/1/data"));
  tests.push_back (test_t ("/shared/7/data"));
  tests.push_back (test_t ("/global/0x1006000000000001/data", sp_t ("global", gpifs::segment::GLOBAL, 0x1006000000000001, "data")));
  tests.push_back (test_t ("/global/4/data"));
  tests.push_back (test_t ("/local/0/data"));
  tests.push_back (test_t ("/local/0x2000000000000004/data", sp_t ("local", gpifs::segment::LOCAL, 0x2000000000000004, "data")));
  tests.push_back (test_t ("/shared/4/0x300000000000012d/data", sp_t ("shared/4", 4, 0x300000000000012d, "data")));
  tests.push_back (test_t ("/shared/4/0x300000000000012e/data", sp_t ("shared/4", 4, 0x300000000000012e, "data")));
  tests.push_back (test_t ("/shared/4/303/data"));
  tests.push_back (test_t ("/0x1006000000000001/data", sp_t (0x1006000000000001, "data")));
  tests.push_back (test_t ("/0x10a0000000000001/data", sp_t (0x10a0000000000001, "data")));
  tests.push_back (test_t ("/5/data"));
  tests.push_back (test_t ("/proc/alloc/data"));

  tests.push_back (test_t ("/globb"));
  tests.push_back (test_t ("/globall"));
  tests.push_back (test_t ("/local/4/da"));

  for  ( test_list::const_iterator t (tests.begin())
       ; t != tests.end()
       ; ++t
       )
    {
      std::cout << "*****" << std::endl;
      std::cout << t->path << ": ";

      gpifs::state::maybe_splitted_path sp (state.split (t->path));

      if (sp)
        {
          std::cout << (state.is_file (*sp) ? "FILE: " : "");
          std::cout << (state.is_directory (*sp) ? "DIRECTORY: " : "");
          std::cout << *sp;
        }
      else
        {
          std::cout << *(state.error_split_get());
        }

      std::cout << std::endl;

      if (!(sp == t->sp))
        {
          std::cout << "FAILURE: expected was ";

          if (t->sp)
            {
              std::cout << *(t->sp);
            }
          else
            {
              std::cout << "-";
            }

          std::cout << std::endl;

          exit (EXIT_FAILURE);
        }
    }

  state.finalize();

  std::cout << "SUCCESS" << std::endl;

  return EXIT_SUCCESS;
}
