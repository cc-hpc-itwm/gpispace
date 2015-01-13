// bernd.loerwald@itwm.fraunhofer.de

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <boost/optional.hpp>
#include <future>
#include <functional>
#include <list>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <network/server.hpp>
#include <rpc/server.hpp>
#include <rpc/client.hpp>
#include <boost/serialization/vector.hpp>
#include <fhg/util/boost/serialization/unordered_map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>


struct binary_and_first_strategy
{
  static std::list<fhg::rpc::endpoints_type> split (fhg::rpc::endpoints_type endpoints)
  {
    if (endpoints.empty())
    {
      return {};
    }
    else if (endpoints.size() == 1)
    {
      return {endpoints};
    }
    else
    {
      return { {endpoints.begin(), endpoints.begin() + endpoints.size() / 2}
             , {endpoints.begin() + endpoints.size() / 2, endpoints.end()}
             };
    }
  }
  static fhg::rpc::endpoint_type select_next_node (fhg::rpc::endpoints_type const& endpoints)
  {
    return endpoints.front();
  }
};

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

  public:
    io_service_with_work_thread_and_stop_on_scope_exit _io_service;
    fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor;
  };

  struct foo_server
  {
    fhg::rpc::service_dispatcher service_dispatcher
      {fhg::rpc::exception::serialization_functions()};

    int const id;
    fhg::rpc::aggregated_service_handler<int (int)> foo_fun
      { service_dispatcher
      , "foo"
      , [&] (int i) { return i + id; }
      , std::bind (&foo_server::is_endpoint, this, std::placeholders::_1)
      , &binary_and_first_strategy::split
      , &binary_and_first_strategy::select_next_node
      , std::bind (&foo_server::io_service, this)
      };

    fhg::rpc::aggregated_service_handler<int()> bar_fun
      { service_dispatcher
      , "bar"
      , [&]() { if (id == 4) throw std::invalid_argument ("4"); return id; }
      , std::bind (&foo_server::is_endpoint, this, std::placeholders::_1)
      , &binary_and_first_strategy::split
      , &binary_and_first_strategy::select_next_node
      , std::bind (&foo_server::io_service, this)
      };

    foo_server (int i) : id (i) {}

    server_for_dispatcher server {service_dispatcher};
    bool is_endpoint (fhg::rpc::endpoint_type endpoint) const
    {
      return endpoint == server.acceptor.local_endpoint();
    }
    io_service_with_work_thread_and_stop_on_scope_exit _io_service;
    boost::asio::io_service& io_service()
    {
      return _io_service.service;
    }
  };
}

int main (int, char**)
{
  foo_server s0 (0);
  foo_server s1 (1);
  foo_server s2 (2);
  foo_server s3 (3);
  foo_server s4 (4);
  foo_server s5 (5);

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;

  {
    fhg::rpc::remote_endpoint rpc_endpoint
      ( io_service_client.service
      , s0.server.acceptor.local_endpoint().address().to_string()
      , s0.server.acceptor.local_endpoint().port()
      , fhg::rpc::exception::serialization_functions()
      );
    fhg::rpc::sync_aggregated_remote_function<int (int)> fun (rpc_endpoint, "foo");

    for ( std::pair<fhg::rpc::endpoint_type, int> kv
        : fun ( { s5.server.acceptor.local_endpoint()
                , s3.server.acceptor.local_endpoint()
                , s2.server.acceptor.local_endpoint()
                , s1.server.acceptor.local_endpoint()
                , s4.server.acceptor.local_endpoint()
                }
              , 100
              )
        )
    {
      std::cerr << "foo: " << kv.first << ": " << kv.second << std::endl;
    }
  }

  try
  {
    fhg::rpc::remote_endpoint rpc_endpoint
      ( io_service_client.service
      , s1.server.acceptor.local_endpoint().address().to_string()
      , s1.server.acceptor.local_endpoint().port()
      , fhg::rpc::exception::serialization_functions()
      );
    fhg::rpc::sync_aggregated_remote_function<int()> fun (rpc_endpoint, "bar");

    for ( std::pair<fhg::rpc::endpoint_type, int> kv
        : fun ( { s1.server.acceptor.local_endpoint()
                , s5.server.acceptor.local_endpoint()
                , s4.server.acceptor.local_endpoint()
                }
              )
        )
    {
      std::cerr << "bar: " << kv.first << ": " << kv.second << std::endl;
    }
  }
  catch (fhg::rpc::aggregated_exception<int> const& ex)
  {
    for (std::pair<fhg::rpc::endpoint_type, int> kv : ex.succeeded)
    {
      std::cerr << "+ bar: " << kv.first << ": " << kv.second << std::endl;
    }
    for (std::pair<fhg::rpc::endpoint_type, std::exception_ptr> ke : ex.failed)
    {
      try
      {
        std::rethrow_exception (ke.second);
      }
      catch (std::exception const& ex)
      {
        std::cerr << "- bar: " << ke.first << ": " << typeid (ex).name() << ": " << ex.what() << std::endl;
      }
    }
  }
}
