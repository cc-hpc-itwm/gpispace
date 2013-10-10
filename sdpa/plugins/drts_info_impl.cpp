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

      std::string worker_to_hostname (std::string const &w)
      {
        const std::string::size_type host_start = w.find ('-') + 1;
        const std::string::size_type host_end = w.find ('-', host_start);

        return w.substr (host_start, host_end-host_start);
      }

      /************************************************************************\
       *                                                                       *
       * internal functions not directly exposed to user code                  *
       *                                                                       *
      \************************************************************************/

      void set_worker_list (std::list<std::string> const &new_worker_list)
      {
        s_get_worker_list () = new_worker_list;
      }
    }
  }
}
