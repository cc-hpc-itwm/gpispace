// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_NETWORK_CONNECTION_HPP
#define FHG_NETWORK_CONNECTION_HPP

#include <boost/asio/generic/stream_protocol.hpp>
#include <boost/asio/strand.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <functional>
#include <list>
#include <mutex>
#include <vector>

namespace fhg
{
  namespace network
  {
    using buffer_type = std::vector<char>;
    using filter_type = std::function<buffer_type (buffer_type)>;
    using handler_type = std::function<void (buffer_type)>;

    struct connection_type : boost::noncopyable
    {
    public:
      connection_type
        ( boost::asio::generic::stream_protocol::socket socket
        , filter_type encrypt
        , filter_type decrypt
        , std::function<void (connection_type*, buffer_type)> on_message
        , std::function<void (connection_type*)> on_disconnect
        );

      template<typename C> void send (C c)
      {
        send (buffer_type (std::begin (c), std::end (c)));
      }
      void send (buffer_type what);

    private:
      void start_read();
      void start_write();

      boost::asio::generic::stream_protocol::socket _socket;
      filter_type _encrypt;
      filter_type _decrypt;
      std::function<void (connection_type*, buffer_type)> _on_message;
      std::function<void (connection_type*)> _on_disconnect;

      std::vector<char> _receive_buffer;
      std::vector<char> _receive_buffer_previous_rest;
      boost::optional<std::vector<char>> _partial_receiving_message;
      boost::optional<std::size_t> _remaining_bytes_for_receiving_message;

      std::mutex _pending_send_mutex;
      std::list<std::vector<char>> _pending_send;

      boost::asio::io_service::strand _receive_strand;
    };
  }
}

#endif
