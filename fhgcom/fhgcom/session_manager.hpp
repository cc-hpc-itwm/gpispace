#ifndef FHG_COM_SESSION_MANAGER_HPP
#define FHG_COM_SESSION_MANAGER_HPP 1

#include <set>

#include <fhglog/fhglog.hpp>

#include <fhgcom/memory.hpp>
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

      virtual ~session_manager () {}

      void add_session (session_ptr session)
      {
        try
        {
          on_add_hook (session);

          sessions_.insert(session);
          DLOG(DEBUG, "added session between " << session->local_endpoint_str() << " and " << session->remote_endpoint_str());
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
        sessions_.erase(session);
        DLOG(DEBUG, "removed session between " << session->local_endpoint_str() << " and " << session->remote_endpoint_str());

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
        DLOG(TRACE, "received " << data.size() << " bytes: " << util::log_raw (data));
        on_data_hook (session, data);
      }

    protected:
      virtual void on_add_hook (session_ptr) {}
      virtual void on_del_hook (session_ptr) {}
      virtual void on_data_hook (session_ptr, const std::string &) {}
    private:
      std::set<session_ptr> sessions_;
    };
  }
}

#endif
