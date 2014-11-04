// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_NETWORK_CLIENT_HPP
#define FHG_NETWORK_CLIENT_HPP

#include <network/connection.hpp>

#include <fhg/util/make_unique.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/io_service.hpp>

#include <functional>
#include <string>

namespace fhg
{
  namespace network
  {
    template<typename Protocol, typename EndpointIterator>
      std::unique_ptr<connection_type> connect_client
        ( boost::asio::io_service& io_service
        , EndpointIterator endpoints
        , filter_type encrypt
        , filter_type decrypt
        , std::function<void (buffer_type)> handler
        , std::function<void (connection_type*)> on_disconnect
        )
    {
      typename Protocol::socket socket (io_service);

      boost::asio::connect (socket, endpoints);

      return util::make_unique<connection_type>
        ( std::move (socket)
        , encrypt
        , decrypt
        , [handler] (connection_type*, buffer_type buffer) { handler (buffer); }
        , on_disconnect
        );
    }

    template<typename Protocol>
      std::unique_ptr<connection_type> connect_client
        ( boost::asio::io_service& io_service
        , std::string host
        , unsigned short port
        , filter_type encrypt
        , filter_type decrypt
        , std::function<void (buffer_type)> handler
        , std::function<void (connection_type*)> on_disconnect
        )
    {
      return connect_client<Protocol>
        ( io_service
        , typename Protocol::resolver (io_service).resolve
          ({host, std::to_string (port), typename Protocol::resolver::query::flags()})
        , encrypt
        , decrypt
        , handler
        , on_disconnect
        );
    }

    template<typename Protocol>
      std::unique_ptr<connection_type> connect_client
        ( boost::asio::io_service& io_service
        , typename Protocol::endpoint endpoint
        , filter_type encrypt
        , filter_type decrypt
        , std::function<void (buffer_type)> handler
        , std::function<void (connection_type*)> on_disconnect
        )
    {
      std::list<typename Protocol::endpoint> endpoints {std::move (endpoint)};

      return connect_client<Protocol>
        ( io_service
        , endpoints.begin()
        , encrypt
        , decrypt
        , handler
        , on_disconnect
        );
    }
  }
}

#endif
