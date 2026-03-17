#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_socket_endpoint.hpp>
#include <gspc/rpc/remote_tcp_endpoint.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_socket_provider.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/util/syscall.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/util/warning.hpp>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <fstream>
#include <future>
#include <initializer_list>
#include <limits>
#include <memory>
#include <string>
#include <thread>

namespace protocol
{
  FHG_RPC_FUNCTION_DESCRIPTION (ping, int (int));
}

namespace
{
  std::size_t available_fds()
  {
    //! \note assume 20 magically lost file descriptors (stdio, boost
    //! asio internal (epoll), ...)
    return gspc::util::suppress_warning::sign_conversion<std::size_t>
      ( gspc::util::syscall::sysconf (_SC_OPEN_MAX)
      , "-1 = infinite = 2^64 -> number of clients limited by ports"
      ) - 20;
  }

  std::size_t available_processes()
  {
    //! \note assume 2000 magically lost processes (probably a low
    //! bound, seeing that even browsers spawn a process/thread per
    //! tab nowadays).
    return gspc::util::suppress_warning::sign_conversion<std::size_t>
      ( gspc::util::syscall::sysconf (_SC_CHILD_MAX)
      , "-1 = infinite = 2^64 -> number of threads spawned limited by fds/ports"
      ) - 2000;
  }

  std::size_t available_ports()
  {
    //! \note CI runners can restrict the ephemeral source port range.
    //! Query the kernel range directly to avoid overestimating.
    std::ifstream range ("/proc/sys/net/ipv4/ip_local_port_range");
    unsigned long lower (0);
    unsigned long upper (0);

    if (range >> lower >> upper && lower <= upper)
    {
      //! \note keep headroom for unrelated traffic and races.
      std::size_t const reserve_ports (256);
      std::size_t const port_count (upper - lower + 1);
      return std::max<std::size_t> (1, port_count - reserve_ports);
    }

    //! \note fallback when /proc is unavailable.
    return std::numeric_limits<uint16_t>::max() - 1024 * 2;
  }

  std::size_t available_autobind_sockets()
  {
    //! \note The actual limit is around 2^20 according to the internet,
    //! but I can't find a value in our 2008 kernel man pages, or a
    //! ulimit. It is very likely greater or equal to the number of
    //! ports due to there being UNIX_PATH_MAX-1 (107) characters for
    //! use in sockaddr_un.sun_path. Simply printing it shows that our
    //! kernel uses a 5 digit base10 number, thus allowing for 99999
    //! anonymous sockets. Due to this all being handweaving, just
    //! assume the same limit as for ports.
    return available_ports();
  }

  std::size_t max_client_count_tcp()
  {
    //! \note assume 1 fd for server accepting, 2 fds per connection
    //! (server + client socket), limit to available ports at most
    return std::min (available_fds() - 1, available_ports()) / 2;
  }

  std::size_t max_client_count_socket()
  {
    //! \note assume 1 fd for server accepting, 2 fds per connection
    //! (server + client socket), limit to available autobind sockets at most
    return std::min (available_fds() - 1, available_autobind_sockets()) / 2;
  }

  std::size_t conservative_thread_limit()
  {
    //! \note CI cgroup and ulimit constraints can be much tighter
    //! than _SC_CHILD_MAX suggests. Keep stress realistic.
    unsigned int const hardware_threads (std::thread::hardware_concurrency());
    std::size_t const fallback_limit (16);
    std::size_t const scaled_hardware_limit
      (std::max<std::size_t> (8, std::size_t (hardware_threads) * 2));

    return std::min<std::size_t>
      (64, hardware_threads ? scaled_hardware_limit : fallback_limit);
  }

  std::size_t max_client_thread_count_tcp()
  {
    return std::min
      ( std::min (max_client_count_tcp(), available_processes())
      , conservative_thread_limit()
      );
  }
  std::size_t max_client_thread_count_socket()
  {
    return std::min
      ( std::min (max_client_count_socket(), available_processes())
      , conservative_thread_limit()
      );
  }
}

namespace data
{
  namespace
  {
    decltype (::boost::unit_test::data::make (std::vector<std::size_t>()))
      values (std::initializer_list<std::size_t> elems)
    {
      return ::boost::unit_test::data::make (std::vector<std::size_t> (elems));
    }
  }
}

BOOST_DATA_TEST_CASE
  ( local_tcp
  , ( data::values ({100, 500, max_client_count_tcp()})
    ^ data::values ({10, 20, max_client_thread_count_tcp()})
    )
  , client_count
  , client_thread_count
  )
{
  BOOST_REQUIRE_LE (client_count, max_client_count_tcp());
  BOOST_REQUIRE_LE (client_thread_count, available_processes());

  std::size_t const rounds (17);

  gspc::util::scoped_boost_asio_io_service_with_threads
    io_service_clients (client_thread_count);
  gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (2);

  gspc::rpc::service_dispatcher service_dispatcher;
  gspc::rpc::service_handler<protocol::ping> start_service
    ( service_dispatcher
    , [] (int i)
      {
        std::this_thread::sleep_for (std::chrono::microseconds (100));
        return i + 1;
      }
    );
  gspc::rpc::service_tcp_provider const server
    (io_service_server, service_dispatcher);
  std::string const endpoint_address
    ( gspc::util::connectable_to_address_string
        (server.local_endpoint().address())
    );

  std::vector<std::unique_ptr<gspc::rpc::remote_tcp_endpoint>> clients;
  std::vector<std::future<int>> futures;

  for (std::size_t i (0); i < client_count; ++i)
  {
    clients.emplace_back
      ( std::make_unique<gspc::rpc::remote_tcp_endpoint>
          (io_service_clients, endpoint_address, server.local_endpoint().port())
      );
    futures.emplace_back
      (gspc::rpc::remote_function<protocol::ping> {*clients.back()} (i));
  }

  std::vector<std::future<int>> futures2;

  std::vector<std::future<int>>* current (&futures);
  std::vector<std::future<int>>* next (&futures2);

  std::size_t round (0);
  for (; round < rounds; ++round)
  {
    for (std::size_t i (0); i < client_count; ++i)
    {
      BOOST_REQUIRE_EQUAL (current->at (i).get(), round + i + 1);
      next->emplace_back
        ( gspc::rpc::remote_function<protocol::ping> {*clients.at (i)}
            (round + 1 + i)
        );
    }
    current->clear();
    std::swap (current, next);
  }

  for (std::size_t i (0); i < client_count; ++i)
  {
    BOOST_REQUIRE_EQUAL (current->at (i).get(), round + i + 1);
  }
}

BOOST_DATA_TEST_CASE
  ( local_socket
  , ( data::values ({100, 500, max_client_count_tcp()})
    ^ data::values ({10, 20, max_client_thread_count_socket()})
    )
  , client_count
  , client_thread_count
  )
{
  BOOST_REQUIRE_LE (client_count, max_client_count_socket());
  BOOST_REQUIRE_LE (client_thread_count, available_processes());

  std::size_t const rounds (17);

  gspc::util::scoped_boost_asio_io_service_with_threads
    io_service_clients (client_thread_count);
  gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (2);

  gspc::rpc::service_dispatcher service_dispatcher;
  gspc::rpc::service_handler<protocol::ping> start_service
    ( service_dispatcher
    , [] (int i)
      {
        std::this_thread::sleep_for (std::chrono::microseconds (100));
        return i + 1;
      }
    );
  gspc::rpc::service_socket_provider const server
    (io_service_server, service_dispatcher);

  std::vector<std::unique_ptr<gspc::rpc::remote_socket_endpoint>> clients;
  std::vector<std::future<int>> futures;

  for (std::size_t i (0); i < client_count; ++i)
  {
    clients.emplace_back
      ( std::make_unique<gspc::rpc::remote_socket_endpoint>
          (io_service_clients, server.local_endpoint())
      );
    futures.emplace_back
      (gspc::rpc::remote_function<protocol::ping> {*clients.back()} (i));
  }

  std::vector<std::future<int>> futures2;

  std::vector<std::future<int>>* current (&futures);
  std::vector<std::future<int>>* next (&futures2);

  std::size_t round (0);
  for (; round < rounds; ++round)
  {
    for (std::size_t i (0); i < client_count; ++i)
    {
      BOOST_REQUIRE_EQUAL (current->at (i).get(), round + i + 1);
      next->emplace_back
        ( gspc::rpc::remote_function<protocol::ping> {*clients.at (i)}
            (round + 1 + i)
        );
    }
    current->clear();
    std::swap (current, next);
  }

  for (std::size_t i (0); i < client_count; ++i)
  {
    BOOST_REQUIRE_EQUAL (current->at (i).get(), round + i + 1);
  }
}
