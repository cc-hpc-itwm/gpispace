// bernd.loerwald@itwm.fraunhofer.de

#include <rpc/client.hpp>

#include <network/client.hpp>

#include <boost/range/adaptor/map.hpp>

#include <mutex>

namespace fhg
{
  namespace rpc
  {
    remote_endpoint::remote_endpoint
        ( boost::asio::io_service& io_service
        , std::string host
        , unsigned short port
        , exception::deserialization_functions functions
        )
      : _deserialization_functions (std::move (functions))
      , _connection
        ( network::connect_client<boost::asio::ip::tcp>
          ( io_service
          , host
          , port
          , [] (network::buffer_type b) { return b; }
          , [] (network::buffer_type b) { return b; }
          , [this] (network::buffer_type b)
          {
            const std::lock_guard<std::mutex> _ (_promises_and_disconnect_mutex);

            packet_header* header ((packet_header*)b.data());

            _promises.at (header->message_id).set_value
              ( network::buffer_type
                (header->buffer, header->buffer + header->buffer_size)
              );
            _promises.erase (header->message_id);
          }
          , [this] (network::connection_type*)
          {
            const std::lock_guard<std::mutex> _ (_promises_and_disconnect_mutex);

            _disconnected = true;
            for ( std::promise<network::buffer_type>& promise
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

    std::future<network::buffer_type>
      remote_endpoint::send_and_receive (network::buffer_type buffer)
    {
      const std::lock_guard<std::mutex> _ (_promises_and_disconnect_mutex);

      if (_disconnected)
      {
        throw std::system_error
          (std::make_error_code (std::errc::connection_aborted));
      }

      uint64_t message_id (++_message_counter);

      packet_header header {message_id, buffer.size()};
      _connection->send
        ({{(char*)&header, (char*)&header + sizeof (header)}, std::move (buffer)});

      _promises.emplace (message_id, std::promise<network::buffer_type>{});
      return _promises.at (message_id).get_future();
    }
  }
}
