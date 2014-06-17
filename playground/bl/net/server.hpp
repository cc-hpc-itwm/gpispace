// bernd.loerwald@itwm.fraunhofer.de

#ifndef PLAYGROUND_BL_NET_SERVER_HPP
#define PLAYGROUND_BL_NET_SERVER_HPP

#include <playground/bl/net/connection.hpp>

#include <fhg/util/make_unique.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/system/system_error.hpp>

#include <functional>
#include <memory>

template<typename Protocol>
class continous_acceptor
{
public:
  continous_acceptor ( typename Protocol::endpoint endpoint
                     , boost::asio::io_service& io_service
                     , filter_type encrypt
                     , filter_type decrypt
                     , std::function<void (connection_type*, buffer_type)> on_message
                     , std::function<void (connection_type*)> on_disconnect
                     , std::function<void (std::unique_ptr<connection_type>)> accept_handler
                     )
    : _acceptor (io_service, endpoint)
    , _encrypt (encrypt)
    , _decrypt (decrypt)
    , _on_message (on_message)
    , _on_disconnect (on_disconnect)
    , _accept_handler (accept_handler)
    , _pending_socket (nullptr)
  {
    accept();
  }

private:
  void accept()
  {
    _pending_socket = fhg::util::make_unique<typename Protocol::socket>
      (_acceptor.get_io_service());

    _acceptor.async_accept
      ( *_pending_socket
      , [this] (boost::system::error_code error)
      {
        if (error)
        {
          throw boost::system::system_error (error);
        }

        _accept_handler
          ( fhg::util::make_unique<connection_type>
            ( std::move (*_pending_socket)
            , _encrypt
            , _decrypt
            , _on_message
            , _on_disconnect
            )
          );

        _pending_socket = nullptr;

        accept();
      }
      );
  }

  typename Protocol::acceptor _acceptor;

  filter_type _encrypt;
  filter_type _decrypt;
  std::function<void (connection_type*, buffer_type)> _on_message;
  std::function<void (connection_type*)> _on_disconnect;
  std::function<void (std::unique_ptr<connection_type>)> _accept_handler;

  std::unique_ptr<typename Protocol::socket> _pending_socket;
};

#endif
