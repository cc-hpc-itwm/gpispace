// bernd.loerwald@itwm.fraunhofer.de

#ifndef PLAYGROUND_BL_NET_CLIENT_HPP
#define PLAYGROUND_BL_NET_CLIENT_HPP

#include <playground/bl/net/connection.hpp>

#include <fhg/util/make_unique.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/io_service.hpp>

#include <functional>
#include <string>

template<typename Protocol>
std::unique_ptr<connection_type> connect_client
  ( std::string host
  , unsigned short port
  , boost::asio::io_service& io_service
  , filter_type encrypt
  , filter_type decrypt
  , std::function<void (buffer_type)> handler
  , std::function<void (connection_type*)> on_disconnect
  )
{
  typename Protocol::socket socket (io_service);

  boost::asio::connect ( socket
                       , typename Protocol::resolver (io_service).resolve
                         ({host, std::to_string (port)})
                       );

  return fhg::util::make_unique<connection_type>
    ( std::move (socket)
    , encrypt
    , decrypt
    , [handler] (connection_type*, buffer_type buffer) { handler (buffer); }
    , on_disconnect
    );
}

#endif
