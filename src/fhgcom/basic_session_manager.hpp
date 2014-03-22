#ifndef FHG_COM_BASIC_SESSION_MANAGER_HPP
#define FHG_COM_BASIC_SESSION_MANAGER_HPP 1

#include <boost/shared_ptr.hpp>

namespace fhg
{
  namespace com
  {
    class session;

    class basic_session_manager
    {
    public:
      typedef boost::shared_ptr<session> session_ptr;
      virtual ~basic_session_manager () {}

      virtual void add_session (session_ptr session) = 0;
      virtual void del_session (session_ptr session) = 0;
      virtual void handle_data (session_ptr session, const std::string & data) = 0;
    };
  }
}

#endif
