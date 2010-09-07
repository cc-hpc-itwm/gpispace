#ifndef FHG_COM_TCP_CLIENT_HPP
#define FHG_COM_TCP_CLIENT_HPP 1

#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/asio.hpp>

namespace fhg
{
  namespace com
  {
    class tcp_client : public boost::enable_shared_from_this<tcp_client>
    {
    public:
      typedef boost::unique_lock<boost::recursive_mutex> lock_t;

      tcp_client ( boost::asio::io_service & io_service
                 , const std::string & host
                 , const std::string & service
                 );

      virtual ~tcp_client ();

      void close ();
      void send ( const std::string & data );
      std::string recv ();
    private:

      void handle_connect ( const boost::system::error_code &
                          , boost::asio::ip::tcp::resolver::iterator
                          );

      void handle_write (const boost::system::error_code &);

      void read_header ();

      void handle_read_header ( const boost::system::error_code &
                              , size_t
                              );

      void handle_read_data ( const boost::system::error_code &
                            , size_t
                            );

      mutable boost::recursive_mutex mutex_;
      boost::condition data_sent_;
      boost::condition data_rcvd_;

      enum { header_length = 8 };
      char inbound_header_[header_length];
      std::vector<char> inbound_data_;

      boost::asio::ip::tcp::socket socket_;
      std::list<std::string> to_send_;
      std::list<std::string> to_recv_;
      bool stopped_;
    };
  }
}

#endif
