// bernd.loerwald@itwm.fraunhofer.de

#ifndef RPC_TEST_HELPERS_HPP
#define RPC_TEST_HELPERS_HPP

#include <rpc/server.hpp>

#include <network/server.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/scoped_thread.hpp>

namespace
{
  struct io_service_with_work_thread_and_stop_on_scope_exit
  {
    io_service_with_work_thread_and_stop_on_scope_exit()
      : service()
      , _work (service)
      , _thread ([this] { service.run(); })
    {}
    ~io_service_with_work_thread_and_stop_on_scope_exit()
    {
      service.stop();
    }
    boost::asio::io_service service;

  private:
    boost::asio::io_service::work const _work;
    boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable> const
      _thread;
  };


  struct server_for_dispatcher
  {
    server_for_dispatcher (fhg::rpc::service_dispatcher& service_dispatcher)
      : _service_dispatcher (service_dispatcher)
      , _connections()
      , _io_service()
      , acceptor
        ( boost::asio::ip::tcp::endpoint (boost::asio::ip::address(), 0)
        , _io_service.service
        , [] (fhg::network::buffer_type buf) { return buf; }
        , [] (fhg::network::buffer_type buf) { return buf; }
        , [this] ( fhg::network::connection_type* connection
                 , fhg::network::buffer_type message
                 )
        {
          _service_dispatcher.dispatch (connection, message);
        }
        , [this] (fhg::network::connection_type* connection)
        {
          _connections.erase
            ( std::find_if
              ( _connections.begin()
              , _connections.end()
              , [&connection]
                (std::unique_ptr<fhg::network::connection_type> const& other)
              {
                return other.get() == connection;
              }
              )
            );
        }
        , [this] (std::unique_ptr<fhg::network::connection_type> connection)
        {
          _connections.emplace_back (std::move (connection));
        }
        )
    {}
  private:
    fhg::rpc::service_dispatcher& _service_dispatcher;
    std::vector<std::unique_ptr<fhg::network::connection_type>> _connections;
    io_service_with_work_thread_and_stop_on_scope_exit _io_service;

  public:
    fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor;
  };
}

#endif
