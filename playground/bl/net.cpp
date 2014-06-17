// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/syscall.hpp>
#include <fhg/util/make_unique.hpp>
#include <fhg/util/num.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/generic/stream_protocol.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <algorithm>
#include <functional>
#include <future>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using buffer_type = std::vector<char>;
using filter_type = std::function<buffer_type (buffer_type)>;
using handler_type = std::function<void (buffer_type)>;

/// connection_type ------------------------------------------------------------

struct connection_type : boost::noncopyable
{
  connection_type ( boost::asio::generic::stream_protocol::socket* socket
                  , filter_type encrypt
                  , filter_type decrypt
                  , std::function<void (connection_type*, buffer_type)> on_message
                  , std::function<void (connection_type*)> on_disconnect
                  )
    : _socket (socket)
    , _encrypt (encrypt)
    , _decrypt (decrypt)
    , _on_message (on_message)
    , _on_disconnect (on_disconnect)
    , _receive_buffer (1 << 16)
    , _receive_buffer_previous_rest()
    , _partial_receiving_message()
    , _remaining_bytes_for_receiving_message()
    , _receive_strand (_socket->get_io_service())
  {
    start_read();
  }

  // connection_type (connection_type&& other)
  //   : _socket (std::move (other._socket))
  //   , _encrypt (other._encrypt)
  //   , _decrypt (other._decrypt)
  //   , _on_message (other._on_message)
  //   , _on_disconnect (other._on_disconnect)
  //   , _receive_buffer (other._receive_buffer)
  //   , _receive_buffer_previous_rest (other._receive_buffer_previous_rest)
  //   , _partial_receiving_message (other._partial_receiving_message)
  //   , _remaining_bytes_for_receiving_message
  //     (other._remaining_bytes_for_receiving_message)
  //   , _pending_send (other._pending_send)
  //   , _receive_strand (other._receive_strand)
  // {
  // }

  void start_read()
  {
    _socket->async_read_some
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

          _receive_buffer.insert ( _receive_buffer.begin()
                                 , _receive_buffer_previous_rest.begin()
                                 , _receive_buffer_previous_rest.end()
                                 );
          transferred += _receive_buffer_previous_rest.size();
          _receive_buffer_previous_rest.clear();

          while (transferred)
          {
            if (_remaining_bytes_for_receiving_message)
            {
              const std::size_t to_eat
                (std::min (*_remaining_bytes_for_receiving_message, transferred));

              _partial_receiving_message->insert
                ( _partial_receiving_message->end()
                , _receive_buffer.begin(), _receive_buffer.begin() + to_eat
                );
              _receive_buffer.erase
                (_receive_buffer.begin(), _receive_buffer.begin() + to_eat);

              *_remaining_bytes_for_receiving_message -= to_eat;
              transferred -= to_eat;
            }
            else
            {
              fhg::util::parse::position_vector_of_char pos (_receive_buffer);
              const std::size_t len (fhg::util::read_size_t (pos));
              if (!pos.end() && *pos == ' ')
              {
                ++pos;

                _receive_buffer.erase
                  (_receive_buffer.begin(), _receive_buffer.begin() + pos.eaten());
                _remaining_bytes_for_receiving_message = len;
                _partial_receiving_message = std::vector<char>();
                transferred -= pos.eaten();
              }
              else
              {
                std::swap (_receive_buffer_previous_rest, _receive_buffer);
                return;
              }
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

  template<typename C>
    void send (C c)
  {
    send (buffer_type (std::begin (c), std::end (c)));
  }
  void send (buffer_type what)
  {
    boost::mutex::scoped_lock _ (_pending_send_mutex);

    const std::string header (std::to_string (what.size()) + ' ');
    what.insert (what.begin(), header.begin(), header.end());

    const bool has_pending (!_pending_send.empty());
    _pending_send.push_back (what);

    if (!has_pending)
    {
      start_write();
    }
  }

  void start_write()
  {
    boost::asio::async_write
      ( *_socket
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

        boost::mutex::scoped_lock _ (_pending_send_mutex);

        _pending_send.pop_front();

        if (!_pending_send.empty())
        {
          start_write();
        }
      }
      );
  }

  std::unique_ptr<boost::asio::generic::stream_protocol::socket> _socket;
  filter_type _encrypt;
  filter_type _decrypt;
  std::function<void (connection_type*, buffer_type)> _on_message;
  std::function<void (connection_type*)> _on_disconnect;

  std::vector<char> _receive_buffer;
  std::vector<char> _receive_buffer_previous_rest;
  boost::optional<std::vector<char>> _partial_receiving_message;
  boost::optional<std::size_t> _remaining_bytes_for_receiving_message;

  boost::mutex _pending_send_mutex;
  std::list<std::vector<char>> _pending_send;

  boost::asio::io_service::strand _receive_strand;
};

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
    ( new boost::asio::generic::stream_protocol::socket (std::move (socket))
    , encrypt
    , decrypt
    , [handler] (connection_type*, buffer_type buffer) { handler (buffer); }
    , on_disconnect
    );
}


/// continous_acceptor ---------------------------------------------------------

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
            ( new boost::asio::generic::stream_protocol::socket
              (std::move (*_pending_socket))
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


/// service_dispatcher ---------------------------------------------------------

struct packet_header
{
  uint64_t message_id;
  std::size_t buffer_size;
  char buffer[0];

  packet_header (uint64_t id, std::size_t size)
    : message_id (id)
    , buffer_size (size)
  {}
};

struct service_dispatcher
{
  void dispatch (connection_type* connection, buffer_type packet) const
  {
    packet_header* header ((packet_header*)packet.data());
    buffer_type message (header->buffer, header->buffer + header->buffer_size);

    fhg::util::parse::position_vector_of_char pos (message);
    const std::size_t len (fhg::util::read_size_t (pos));
    fhg::util::parse::require::require (pos, ' ');

    buffer_type response (_handlers.at (pos.eat (len)) (pos));

    packet_header response_header (header->message_id, response.size());
    buffer_type response_packet (response);
    response_packet.insert ( response_packet.begin()
                           , (char*)&response_header
                           , (char*)&response_header + sizeof (response_header)
                           );

    connection->send (response_packet);
  }

  std::unordered_map
    < std::string
    , std::function<buffer_type (fhg::util::parse::position&)>
    > _handlers;
};

/// service_handler ------------------------------------------------------------

struct service_handler : boost::noncopyable
{
  service_handler ( service_dispatcher& manager
                  , std::string name
                  , std::function<buffer_type (fhg::util::parse::position&)> handler
                  )
    : _manager (manager)
    , _name (name)
  {
    if (_manager._handlers.find (_name) != _manager._handlers.end())
    {
      throw std::runtime_error ("already have service with name " + _name);
    }
    _manager._handlers.insert (std::make_pair (_name, handler));
  }
  ~service_handler()
  {
    _manager._handlers.erase (_name);
  }

private:
  service_dispatcher& _manager;
  std::string _name;
};


/// thunk ----------------------------------------------------------------------

namespace
{
  template<std::size_t...> struct indices;

  template<std::size_t, typename> struct make_indices_impl;
  template<std::size_t N, std::size_t... Indices>
    struct make_indices_impl<N, indices<Indices...>>
  {
    using type = typename make_indices_impl<N - 1, indices<N, Indices...>>::type;
  };
  template<std::size_t... Indices>
    struct make_indices_impl<0, indices<Indices...>>
  {
    using type = indices<0, Indices...>;
  };

  //! \note create indices<0, …, Size - 1>
  //! \note potentially slow for large N, and limited by -ftemplate-depth.
  template<std::size_t Size>
    using make_indices = typename make_indices_impl<Size - 1, indices<>>::type;

  template<typename> struct apply_tuple_impl;
  template<std::size_t... Indices>
    struct apply_tuple_impl<indices<Indices...>>
  {
    template<typename Op, typename... OpArgs>
      static typename std::result_of<Op (OpArgs...)>::type
        apply (Op&& op, std::tuple<OpArgs...>&& t)
    {
      return op (std::forward<OpArgs> (std::get<Indices> (t))...);
    }
  };

  //! \note call op (t...)
  template<typename Op, typename... OpArgs>
    typename std::result_of<Op (OpArgs...)>::type
      apply_tuple (Op&& op, std::tuple<OpArgs...>&& t)
  {
    return apply_tuple_impl<make_indices<sizeof... (OpArgs)>>::apply
      (std::forward<Op> (op), std::forward<std::tuple<OpArgs...>> (t));
  }

  //! \note Alternative to serializing whole tuple

  // template<typename, typename> struct unwrap_arguments_impl;
  // template<typename arguments_type, std::size_t... Indices>
  //   struct unwrap_arguments_impl<arguments_type, indices<Indices...>>
  // {
  //   template<size_t i>
  //     using argument_type = typename std::tuple_element<i, arguments_type>::type;

  //   static arguments_type apply (fhg::util::parse::position& buffer)
  //   {
  //     //! \todo Bug in gcc <= 4.9 (?): order not sequential. Needs to
  //     //! be assembled recursively as workaround.
  //     return arguments_type {unwrap<argument_type<Indices>> (buffer)...};
  //   }
  // };

  // template<typename arguments_type>
  //   arguments_type unwrap_arguments (fhg::util::parse::position& buffer)
  // {
  //   return unwrap_arguments_impl
  //     <arguments_type, make_indices<std::tuple_size<arguments_type>::value>>
  //     ::apply (buffer);
  // }

  template<typename arguments_type>
    arguments_type unwrap_arguments (fhg::util::parse::position& buffer)
  {
    //! \todo extract istream directly from buffer?

    const std::size_t len (fhg::util::read_size_t (buffer));
    fhg::util::parse::require::require (buffer, ' ');
    const std::string blob (buffer.eat (len));
    std::istringstream is (blob);

    boost::archive::text_iarchive ia (is);
    arguments_type args;
    ia & args;

    return args;
  }

  template<std::size_t, std::size_t> struct serialize_impl;
  template<std::size_t N>
    struct serialize_impl<N, N>
  {
    template<typename Archive, typename... Elements>
      static Archive& apply (Archive& ar, std::tuple<Elements...> const&)
    {
      return ar;
    }
  };
  template<std::size_t Index, std::size_t Count>
    struct serialize_impl
  {
    template<typename Archive, typename... Elements>
      static Archive& apply (Archive& ar, std::tuple<Elements...> const& t)
    {
      ar & std::get<Index> (t);
      return serialize_impl<Index + 1, Count>::apply (ar, t);
    }
    template<typename Archive, typename... Elements>
      static Archive& apply (Archive& ar, std::tuple<Elements...>& t)
    {
      ar & std::get<Index> (t);
      return serialize_impl<Index + 1, Count>::apply (ar, t);
    }
  };
}

namespace boost
{
  namespace serialization
  {
    template<typename Archive, typename... Elements>
      Archive& serialize
        (Archive& ar, std::tuple<Elements...>& t, const unsigned int)
    {
      return serialize_impl<0, sizeof... (Elements)>::apply (ar, t);
    }
    template<typename Archive, typename... Elements>
      Archive& serialize
        (Archive& ar, std::tuple<Elements...> const& t, const unsigned int)
    {
      return serialize_impl<0, sizeof... (Elements)>::apply (ar, t);
    }
  }
}

template<typename> struct thunk;
template<typename R, typename... Args>
  struct thunk<R (Args...)>
{
private:
  using function_type = std::function<R (Args...)>;
  using result_type = R;
  using arguments_type = std::tuple<Args...>;

public:
  thunk (function_type fun)
    : _fun (fun)
  {}

  buffer_type operator() (fhg::util::parse::position& buffer)
  {
    result_type ret (apply_tuple (_fun, unwrap_arguments<arguments_type> (buffer)));

    std::ostringstream os;
    boost::archive::text_oarchive oa (os);
    oa & ret;
    const std::string os_str (os.str());
    return buffer_type (os_str.begin(), os_str.end());
  }

private:
  function_type _fun;
};
template<typename... Args>
  struct thunk<void (Args...)>
{
private:
  using function_type = std::function<void (Args...)>;
  using arguments_type = std::tuple<Args...>;

public:
  thunk (function_type fun)
    : _fun (fun)
  {}

  buffer_type operator() (fhg::util::parse::position& buffer)
  {
    apply_tuple (_fun, unwrap_arguments<arguments_type> (buffer));

    std::ostringstream os;
    boost::archive::text_oarchive oa (os);
    const std::string os_str (os.str());
    return buffer_type (os_str.begin(), os_str.end());
  }

private:
  function_type _fun;
};


/// remote_endpoint ------------------------------------------------------------

struct remote_endpoint
{
  remote_endpoint ( boost::asio::io_service& io_service
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
                  ( std::make_error_code (std::errc::connection_aborted))
                )
              );
          }
        }
        )
      )
    , _disconnected (false)
    , _message_counter (0)
  {}

  template<typename C>
    std::future<buffer_type> send_and_receive (C c)
  {
    return send_and_receive (buffer_type (std::begin (c), std::end (c)));
  }
  std::future<buffer_type> send_and_receive (buffer_type buffer)
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

  std::unique_ptr<connection_type> _connection;

  mutable std::mutex _promises_and_disconnect_mutex;
  std::unordered_map<uint64_t, std::promise<buffer_type>> _promises;
  bool _disconnected;

  uint64_t _message_counter;
};


/// remote_function ------------------------------------------------------------

template<typename T> T deserialize_from_buffer (buffer_type buffer)
{
  const std::string blob (buffer.begin(), buffer.end());
  std::istringstream is (blob);
  boost::archive::text_iarchive ia (is);
  T value;
  ia & value;
  return value;
}
template<> void deserialize_from_buffer (buffer_type buffer)
{
  const std::string blob (buffer.begin(), buffer.end());
  std::istringstream is (blob);
  boost::archive::text_iarchive ia (is);
  if (!is.eof())
  {
    throw std::logic_error ("return value for void function");
  }
}

std::string prepend_size (std::string str)
{
  return std::to_string (str.size()) + ' ' + str;
}

template<typename> struct remote_function;
template<typename R, typename... Args>
  struct remote_function<R (Args...)>
{
public:
  remote_function (remote_endpoint& endpoint, std::string function)
    : _endpoint (endpoint)
    , _function (function)
  {
    //! \todo check that function with signature exists
  }

  std::future<R> operator() (Args... args)
  {
    std::tuple<Args...> arguments (args...);

    std::ostringstream args_stringstream;
    boost::archive::text_oarchive oa (args_stringstream);
    oa & arguments;

    return std::async
      ( std::launch::async
      , [] (std::future<buffer_type>&& buffer)
      {
        buffer_type buf (buffer.get());
        return deserialize_from_buffer<R> (buf);
      }
      , std::move
        ( _endpoint.send_and_receive
          (prepend_size (_function) + prepend_size (args_stringstream.str()))
        )
      );
  }

private:
  remote_endpoint& _endpoint;
  std::string _function;
};

template<typename> struct sync_remote_function;
template<typename R, typename... Args>
  struct sync_remote_function<R (Args...)>
{
public:
  sync_remote_function (remote_endpoint& endpoint, std::string function)
    : _function (endpoint, function)
  {}

  R operator() (Args... args)
  {
    return _function (args...).get();
  }

private:
  remote_function<R (Args...)> _function;
};


/// rif ------------------------------------------------------------------------

#include <spawn.h>

struct rif
{
  service_dispatcher _service_dispatcher;
  service_handler _start_service;
  service_handler _stop_service;

  std::vector<std::unique_ptr<connection_type>> _connections;

  continous_acceptor<boost::asio::ip::tcp> _acceptor;

  rif (unsigned short port, boost::asio::io_service& io_service)
    : _service_dispatcher()
    , _start_service ( _service_dispatcher
                     , "start"
                     , thunk<pid_t (std::string, std::vector<std::string>)>
                       (std::bind (&rif::start, this, std::placeholders::_1, std::placeholders::_2))
                     )
    , _stop_service ( _service_dispatcher
                    , "stop"
                    , thunk<void (std::size_t)>
                      (std::bind (&rif::stop, this, std::placeholders::_1))
                    )
    , _acceptor ( boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4(), port)
                , io_service
                , [] (buffer_type buf) { return buf; }
                , [] (buffer_type buf) { return buf; }
                , [this] (connection_type* connection, buffer_type message)
                {
                  _service_dispatcher.dispatch (connection, message);
                }
                , [this] (connection_type* connection)
                {
                  _connections.erase
                    ( std::find_if
                      ( _connections.begin()
                      , _connections.end()
                      , [&connection]
                        (std::unique_ptr<connection_type> const& other)
                      {
                        return other.get() == connection;
                      }
                      )
                    );
                }
                , [this] (std::unique_ptr<connection_type> connection)
                {
                  _connections.push_back (std::move (connection));
                }
                )
  {}

  pid_t start (std::string filename, std::vector<std::string> arguments)
  {
    sleep (1);
    pid_t pid (fhg::syscall::fork());
    if (pid)
    {
      return pid;
    }
    else
    {
      std::vector<char> argv_buffer;
      std::vector<char*> argv;

      {
        arguments.insert (arguments.begin(), filename);
        std::vector<std::size_t> argv_offsets;
        for (std::string arg : arguments)
        {
          std::size_t pos (argv_buffer.size());
          argv_buffer.resize (argv_buffer.size() + arg.size() + 1);
          std::copy (arg.begin(), arg.end(), argv_buffer.data() + pos);
          argv_buffer[argv_buffer.size() - 1] = '\0';
          argv_offsets.push_back (pos);
        }
        for (std::size_t offset : argv_offsets)
        {
          argv.push_back (argv_buffer.data() + offset);
        }
        argv.push_back (nullptr);
      }

      try
      {
        fhg::syscall::execve (argv[0], argv.data(), nullptr);
      }
      catch (boost::system::system_error const&)
      {
        _exit (127);
      }
    }
  }
  void stop (pid_t pid)
  {
    fhg::syscall::kill (pid, 9);
  }
};


/// main -----------------------------------------------------------------------

int main (int argc, char** argv)
{
  boost::asio::io_service io_service;

  if (argc > 1)
  {
    boost::asio::io_service::work io_service_work_ (io_service);
    const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      io_service_thread ([&io_service]() { io_service.run(); });

    remote_endpoint endpoint (io_service, "localhost", 13331);

    remote_function<pid_t (std::string, std::vector<std::string>)> start
      (endpoint, "start");
    sync_remote_function<void (pid_t)> stop
      (endpoint, "stop");

    std::vector<std::string> args;
    for (int i (2); i < argc; ++i)
    {
      args.push_back (argv[i]);
    }
    std::future<pid_t> future_pid (start (argv[1], args));

    std::future_status status;
    do
    {
      status = future_pid.wait_for (std::chrono::milliseconds (500));
      if (status == std::future_status::deferred)
      {
        std::cout << "deferred\n";
      }
      else if (status == std::future_status::timeout)
      {
        std::cout << "timeout\n";
      }
      else if (status == std::future_status::ready)
      {
        std::cout << "ready!\n";
      }
    }
    while (status != std::future_status::ready);

    std::this_thread::sleep_for (std::chrono::seconds (2));

    stop (future_pid.get());

    io_service.stop();

    return 0;
  }

  const rif _ (13331, io_service);
  const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
    io_service_thread ([&io_service]() { io_service.run(); });

  return 0;
}
