// bernd.loerwald@itwm.fraunhofer.de

#include <playground/bl/rpc/client.hpp>

#include <playground/bl/net/client.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/range/adaptor/map.hpp>

#include <mutex>

/// remote_endpoint ------------------------------------------------------------

remote_endpoint::remote_endpoint ( boost::asio::io_service& io_service
                                 , std::string host
                                 , unsigned short port
                                 )
  : _connection
    ( connect_client<boost::asio::ip::tcp>
      ( host, port
      , io_service
      , [] (buffer_type b) { return b; }
      , [] (buffer_type b) { return b; }
      , [this] (buffer_type b)
      {
        const std::lock_guard<std::mutex> _ (_promises_and_disconnect_mutex);

        packet_header* header ((packet_header*)b.data());

        _promises.at (header->message_id).set_value
          ( buffer_type
            (header->buffer, header->buffer + header->buffer_size)
          );
        _promises.erase (header->message_id);
      }
      , [this] (connection_type*)
      {
        const std::lock_guard<std::mutex> _ (_promises_and_disconnect_mutex);

        _disconnected = true;
        for ( std::promise<buffer_type>& promise
            : _promises | boost::adaptors::map_values
            )
        {
          promise.set_exception
            ( std::make_exception_ptr
              ( std::system_error
                (std::make_error_code (std::errc::connection_aborted))
              )
            );
        }
      }
      )
    )
  , _disconnected (false)
  , _message_counter (0)
{}

std::future<buffer_type> remote_endpoint::send_and_receive (buffer_type buffer)
{
  const std::lock_guard<std::mutex> _ (_promises_and_disconnect_mutex);

  if (_disconnected)
  {
    throw std::system_error
      (std::make_error_code (std::errc::connection_aborted));
  }

  uint64_t message_id (++_message_counter);

  packet_header header {message_id, buffer.size()};

  buffer_type packet (buffer);
  packet.insert
    (packet.begin(), (char*)&header, (char*)&header + sizeof (header));

  _promises.emplace (message_id, std::promise<buffer_type>{});

  _connection->send (packet);

  return _promises.at (message_id).get_future();
}
