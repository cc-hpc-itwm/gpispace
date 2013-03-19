#ifndef GSPC_NET_SERVER_SERVICE_DEMUX_HPP
#define GSPC_NET_SERVER_SERVICE_DEMUX_HPP

#include <string>

#include <boost/function.hpp>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>

#include <gspc/net/frame_fwd.hpp>
#include <gspc/net/service/handler.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      class service_demux_t
      {
      public:
        service_demux_t ();

        int handle ( std::string const &dst
                   , gspc::net::service::handler_t
                   );

        int handle_request ( std::string const &dst
                           , frame const & rqst
                           , frame & rply
                           );
      private:
        service_demux_t (service_demux_t const &);
        service_demux_t & operator= (service_demux_t const &);

        typedef boost::shared_lock<boost::shared_mutex> shared_lock;
        typedef boost::unique_lock<boost::shared_mutex> unique_lock;

        typedef boost::unordered_map< std::string
                                    , gspc::net::service::handler_t
                                    > handler_map_t;

        mutable boost::shared_mutex m_mutex;
        handler_map_t               m_handler_map;
      };
    }
  }
}

#endif
