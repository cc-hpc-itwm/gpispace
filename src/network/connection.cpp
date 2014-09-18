// bernd.loerwald@itwm.fraunhofer.de

#include <network/connection.hpp>

#include <fhg/util/num.hpp>
#include <fhg/util/parse/position.hpp>

#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/system_error.hpp>

namespace fhg
{
  namespace network
  {
    namespace protocol
    {
      struct packet_header
      {
        uint64_t size;
      };
    }

    connection_type::connection_type
        ( boost::asio::generic::stream_protocol::socket socket
        , filter_type encrypt
        , filter_type decrypt
        , std::function<void (connection_type*, buffer_type)> on_message
        , std::function<void (connection_type*)> on_disconnect
        )
      : _socket (std::move (socket))
      , _encrypt (encrypt)
      , _decrypt (decrypt)
      , _on_message (on_message)
      , _on_disconnect (on_disconnect)
      , _receive_buffer (1 << 16)
      , _receive_buffer_previous_rest()
      , _partial_receiving_message()
      , _remaining_bytes_for_receiving_message()
      , _receive_strand (_socket.get_io_service())
    {
      start_read();

      _socket.set_option (boost::asio::ip::tcp::no_delay (true));
    }

    void connection_type::start_read()
    {
      _socket.async_read_some
        ( boost::asio::buffer (_receive_buffer)
        , _receive_strand.wrap
        ( [this] ( const boost::system::error_code & error
                 , std::size_t transferred
                 )
        {
          if (error)
          {
            if (error == boost::asio::error::eof)
            {
              return _on_disconnect (this);
            }
            else if (error == boost::asio::error::operation_aborted)
            {
              //! \note Ignore: This is fine: dtor tells read to
              //! cancel. we do not need to clean up anything, as
              //! everything is scoped. just silently stop the thread
              return;
            }
            else
            {
              throw boost::system::system_error (error);
            }
          }

          std::vector<char> data;
          data.reserve (_receive_buffer_previous_rest.size() + transferred);
          data.insert ( data.begin()
                      , _receive_buffer_previous_rest.begin()
                      , _receive_buffer_previous_rest.end()
                      );
          data.insert ( data.end()
                      , _receive_buffer.begin()
                      , _receive_buffer.begin() + transferred
                      );

          transferred += _receive_buffer_previous_rest.size();

          std::vector<char>::const_iterator pos (data.begin());

          while (transferred)
          {
            if (_remaining_bytes_for_receiving_message)
            {
              const std::size_t to_eat
                (std::min (*_remaining_bytes_for_receiving_message, transferred));

              _partial_receiving_message->insert
                (_partial_receiving_message->end(), pos, pos + to_eat);
              pos += to_eat;

              *_remaining_bytes_for_receiving_message -= to_eat;
              transferred -= to_eat;
            }
            else if (transferred >= sizeof (protocol::packet_header))
            {
              protocol::packet_header const header
                ( *static_cast<protocol::packet_header const*>
                  (static_cast<void const*> (&*pos))
                );

              pos += sizeof (protocol::packet_header);

              _remaining_bytes_for_receiving_message = header.size;
              _partial_receiving_message = std::vector<char>();
              transferred -= sizeof (protocol::packet_header);
            }
            else
            {
              std::vector<char> (pos, data.cend()).swap
                (_receive_buffer_previous_rest);
              break;
            }

            if ( _remaining_bytes_for_receiving_message
               && *_remaining_bytes_for_receiving_message == 0
               )
            {
              _on_message (this, _decrypt (*_partial_receiving_message));
              _partial_receiving_message = boost::none;
              _remaining_bytes_for_receiving_message = boost::none;
            }
          }

          start_read();
        }
        )
        );
    }

    void connection_type::send (std::initializer_list<buffer_type> list)
    {
      protocol::packet_header const header
        { std::accumulate
            ( list.begin()
            , list.end()
            , static_cast<uint64_t> (0)
            , [] (uint64_t s, buffer_type const& x) { return s + x.size(); }
            )
        };

      std::lock_guard<std::mutex> const _ (_pending_send_mutex);

      const bool has_pending (!_pending_send.empty());

      //! \note We trade avoiding concat (header, list…) with possibly
      //! multiple network transfers. It may be better to just concat
      //! buffers and transfer once.

      _pending_send.emplace_back ((char*)&header, (char*)&header + sizeof (header));

      for (buffer_type const& buffer : list)
      {
        _pending_send.emplace_back (std::move (buffer));
      }

      if (!has_pending)
      {
        start_write();
      }
    }

    void connection_type::start_write()
    {
      boost::asio::async_write
        ( _socket
        , boost::asio::buffer (_pending_send.front())
        , [this] (boost::system::error_code error, std::size_t /*written*/)
        {
          //! \note written is only != expected, if error.
          if (error)
          {
            if (error == boost::asio::error::eof)
            {
              _on_disconnect (this);
              return;
            }

            throw boost::system::system_error (error);
          }

          std::lock_guard<std::mutex> const _ (_pending_send_mutex);

          _pending_send.pop_front();

          if (!_pending_send.empty())
          {
            start_write();
          }
        }
        );
    }
  }
}
