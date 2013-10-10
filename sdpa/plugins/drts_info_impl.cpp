#include "drts_info.hpp"
#include "drts_info_impl.hpp"

namespace gspc
{
  namespace drts
  {
    namespace info
    {
      static std::list<std::string> &s_get_worker_list ()
      {
        static std::list<std::string> worker_list;
        return worker_list;
      }

      std::list<std::string> const &worker_list ()
      {
        return s_get_worker_list ();
      }

      void set_worker_list (std::list<std::string> const &new_worker_list)
      {
        s_get_worker_list () = new_worker_list;
      }
    }
  }
}
