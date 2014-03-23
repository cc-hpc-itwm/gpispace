#ifndef FHG_COM_SESSION_MANAGER_HPP
#define FHG_COM_SESSION_MANAGER_HPP 1

#include <fhgcom/session.hpp>

#include <boost/shared_ptr.hpp>

namespace fhg
{
  namespace com
  {
    class session;

    class session_manager
    {
    public:
      typedef boost::shared_ptr<session> session_ptr;

      void add_session (session_ptr session)
      {
      }

      void del_session (session_ptr session)
      {
      }

      void handle_data (session_ptr session, const std::string & data)
      {
        on_data_hook (session, data);
      }

      virtual ~session_manager() {}
    protected:
      virtual void on_data_hook (session_ptr, const std::string &) = 0;
    };
  }
}

#endif
