#include "convenience.hpp"
#include "manager.hpp"
#include "util.hpp"

namespace gspc
{
  namespace rif
  {
    proc_t exec (manager_t & mgr, std::string const &cmdline)
    {
      size_t c;
      argv_t argv;
      parse (cmdline, argv, c);
      if (c != cmdline.size ())
      {
        return -EINVAL;
      }
      return mgr.exec (argv);
    }
  }
}
