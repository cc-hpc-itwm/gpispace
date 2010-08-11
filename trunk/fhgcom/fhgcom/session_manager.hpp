#ifndef FHG_COM_SESSION_MANAGER_HPP
#define FHG_COM_SESSION_MANAGER_HPP 1

#include <fhgcom/basic_session.hpp>
#include <fhgcom/memory.hpp>

#include <set>

namespace fhg
{
  namespace com
  {
    class session_manager
    {
    public:
      typedef shared_ptr<basic_session> basic_session_ptr;

      void add (basic_session_ptr session)
      {
        LOG(INFO, "adding session between " << session->local_endpoint_str() << " and " << session->remote_endpoint_str());
        sessions_.insert(session);
      }

      void del (basic_session_ptr session)
      {
        LOG(INFO, "removing session between " << session->local_endpoint_str() << " and " << session->remote_endpoint_str());
        sessions_.erase(session);
      }
    private:
      std::set<basic_session_ptr> sessions_;
    };
  }
}

#endif
