#ifndef GSPC_NET_SERVER_SERVICE_DEMUX_HPP
#define GSPC_NET_SERVER_SERVICE_DEMUX_HPP

#include <string>

#include <boost/function.hpp>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include <gspc/net/service/handler.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      class service_demux_t : boost::noncopyable
      {
      public:
        service_demux_t ();

        int handle ( std::string const &dst
                   , gspc::net::service::handler_t
                   );
        int unhandle (std::string const &dst);

        int handle_request ( std::string const &dst
                           , frame const & rqst
                           , user_ptr user
                           );
      private:
        void do_service_help ( std::string const &dst
                             , gspc::net::frame const &rqst
                             , gspc::net::user_ptr user
                             );

        typedef boost::shared_lock<boost::shared_mutex> shared_lock;
        typedef boost::unique_lock<boost::shared_mutex> unique_lock;

        typedef boost::unordered_map< std::string
                                    , gspc::net::service::handler_t
                                    > handler_map_t;

        mutable boost::shared_mutex m_mutex;
        handler_map_t               m_handler_map;
      };

      struct scoped_service_handler : boost::noncopyable
      {
        scoped_service_handler ( std::string name
                               , gspc::net::service::handler_t
                               , gspc::net::server::service_demux_t&
                               );
        ~scoped_service_handler();

      private:
        std::string _name;
        gspc::net::server::service_demux_t& _service_demux;
      };
    }
  }
}

#endif
