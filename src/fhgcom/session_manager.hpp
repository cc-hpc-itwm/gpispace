#ifndef FHG_COM_SESSION_MANAGER_HPP
#define FHG_COM_SESSION_MANAGER_HPP 1

#include <set>

#include <fhglog/fhglog.hpp>

#include <fhgcom/memory.hpp>
#include <boost/thread.hpp>
#include <fhgcom/basic_session_manager.hpp>
#include <fhgcom/util/to_hex.hpp>

#include <fhgcom/session.hpp>

namespace fhg
{
  namespace com
  {
    class session_manager : public basic_session_manager
    {
    public:
      typedef session session_type;
      typedef shared_ptr<session_type> session_ptr;

      void add_session (session_ptr session)
      {
        boost::mutex::scoped_lock const _ (_mutex_sessions);
        sessions_.insert(session);
      }

      void del_session (session_ptr session)
      {
        boost::mutex::scoped_lock const _ (_mutex_sessions);
        sessions_.erase(session);
      }

      void handle_data (session_ptr session, const std::string & data)
      {
        on_data_hook (session, data);
      }

    protected:
      virtual void on_data_hook (session_ptr, const std::string &) {}
    private:
      boost::mutex          _mutex_sessions;
      std::set<session_ptr> sessions_;
    };
  }
}

#endif