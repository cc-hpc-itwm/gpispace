#ifndef FHG_COM_SESSION_MANAGER_HPP
#define FHG_COM_SESSION_MANAGER_HPP 1

#include <set>

#include <fhglog/fhglog.hpp>

#include <fhgcom/memory.hpp>
#include <boost/thread.hpp>
#include <fhgcom/basic_session_manager.hpp>
#include <fhgcom/util/to_hex.hpp>

namespace fhg
{
  namespace com
  {
    template <typename Session>
    class session_manager : public basic_session_manager<Session>
    {
    public:
      typedef Session session_type;
      typedef shared_ptr<session_type> session_ptr;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      void add_session (session_ptr session)
      {
        try
        {
          on_add_hook (session);

          {
            lock_type lock (m_mutex);
            sessions_.insert(session);
          }
        } catch (std::exception const & ex)
        {
          // TODO: fine grained exceptions
          //    maybe add something like permission denied...
          LOG(ERROR, "on_add_hook failed: " << ex.what());
          // throw?
        }
      }

      void del_session (session_ptr session)
      {
        {
          lock_type lock (m_mutex);
          sessions_.erase(session);
        }

        try
        {
          on_del_hook (session);
        } catch (std::exception const & ex)
        {
          LOG(ERROR, "on_del_hook failed: " << ex.what());
        }
      }

      void handle_data (session_ptr session, const std::string & data)
      {
        on_data_hook (session, data);
      }

    protected:
      virtual void on_add_hook (session_ptr) {}
      virtual void on_del_hook (session_ptr) {}
      virtual void on_data_hook (session_ptr, const std::string &) {}
    private:
      mutex_type            m_mutex;
      std::set<session_ptr> sessions_;
    };
  }
}

#endif
