#ifndef FHG_COM_TCP_CLIENT_HPP
#define FHG_COM_TCP_CLIENT_HPP 1

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/asio.hpp>

namespace fhg
{
  namespace com
  {
    class tcp_client
    {
    public:
      typedef boost::unique_lock<boost::recursive_mutex> lock_t;

      tcp_client ();

      virtual ~tcp_client ();

      void start ( const std::string & host
                 , const std::string & port
                 , boost::posix_time::time_duration timeout = boost::posix_time::seconds(10)
                 );

      void stop ();

      void send ( const std::string & data
                , boost::posix_time::time_duration timeout = boost::posix_time::seconds(30)
                );
      std::string recv (boost::posix_time::time_duration timeout = boost::posix_time::pos_infin);
    private:
      void check_deadline ();

      mutable boost::recursive_mutex mutex_;

      enum { header_length = 8 };

      boost::asio::io_service io_service_;
      boost::asio::ip::tcp::socket socket_;
      boost::asio::deadline_timer deadline_;
    };
  }
}

#endif
