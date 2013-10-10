#ifndef GSPC_DRTS_INFO_HPP
#define GSPC_DRTS_INFO_HPP

#include <string>
#include <list>

namespace gspc
{
  namespace drts
  {
    namespace info
    {
      std::list<std::string> const &get_worker_list ();
      void set_worker_list (std::list<std::string> const &);
    }
  }
}

#endif
