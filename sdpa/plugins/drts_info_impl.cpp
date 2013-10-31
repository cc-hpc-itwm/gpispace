#include "drts_info.hpp"
#include "drts_info_impl.hpp"
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace gspc
{
  namespace drts
  {
    namespace info
    {
      struct drts_info_t
      {
      public:
        void set_worker_list (std::list<std::string> const &wl)
        {
          boost::unique_lock<boost::shared_mutex> lock (m_mutex);
          m_worker_list = wl;
        }

        std::list<std::string> get_worker_list () const
        {
          boost::shared_lock<boost::shared_mutex> lock (m_mutex);
          return m_worker_list;
        }
      private:
        mutable boost::shared_mutex m_mutex;

        std::list<std::string> m_worker_list;
      };

      static drts_info_t & s_get_drts_info ()
      {
        static drts_info_t di;
        return di;
      }

      std::list<std::string> worker_list ()
      {
        return s_get_drts_info ().get_worker_list ();
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
        s_get_drts_info ().set_worker_list (new_worker_list);
      }
    }
  }
}
