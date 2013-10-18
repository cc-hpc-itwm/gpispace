#ifndef GSPC_NET_CLIENT_HPP
#define GSPC_NET_CLIENT_HPP

#include <string>
#include <gspc/net/frame_fwd.hpp>
#include <gspc/net/frame_handler_fwd.hpp>

#include <boost/signal.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace gspc
{
  namespace net
  {
    class client_t
    {
    public:
      virtual ~client_t () {}

      boost::signal<void (boost::system::error_code const &)> onError;
      boost::signal<void (frame const &)>                     onFrame;

      virtual int start () = 0;
      virtual int start (const boost::posix_time::time_duration) = 0;
      virtual int stop () = 0;

      virtual std::string const & get_private_queue () const = 0;

      virtual bool is_connected () const = 0;

      virtual int connect () = 0;
      virtual int connect (const boost::posix_time::time_duration) = 0;
      virtual int disconnect () = 0;

      virtual void set_frame_handler (frame_handler_t &) = 0;

      virtual int send_raw (frame const &) = 0;

      virtual int request ( std::string const &dst
                          , std::string const &body
                          , frame &rply
                          , const boost::posix_time::time_duration
                          ) = 0;
      virtual int request ( frame const &rqst, frame &rply
                          , const boost::posix_time::time_duration
                          ) = 0;
      virtual int send ( std::string const & dst
                       , std::string const & body
                       ) = 0;
      virtual int send_sync ( frame const &
                            , const boost::posix_time::time_duration
                            ) = 0;
      virtual int send_sync ( std::string const & dst
                            , std::string const & body
                            , const boost::posix_time::time_duration
                            ) = 0;
      virtual int subscribe ( std::string const &dest
                            , std::string const &id
                            ) = 0;
      virtual int unsubscribe (std::string const &id) = 0;
    };
  }
}

#endif
