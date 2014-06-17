// bernd.loerwald@itwm.fraunhofer.de

#ifndef PLAYGROUND_BL_RPC_CLIENT_HPP
#define PLAYGROUND_BL_RPC_CLIENT_HPP

#include <playground/bl/rpc/common.hpp>

#include <playground/bl/net/connection.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio/io_service.hpp>

#include <cstdint>
#include <future>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

struct remote_endpoint
{
public:
  remote_endpoint ( boost::asio::io_service& io_service
                  , std::string host
                  , unsigned short port
                  );

  template<typename C>
    std::future<buffer_type> send_and_receive (C c)
  {
    return send_and_receive (buffer_type (std::begin (c), std::end (c)));
  }
  std::future<buffer_type> send_and_receive (buffer_type buffer);

private:
  std::unique_ptr<connection_type> _connection;

  mutable std::mutex _promises_and_disconnect_mutex;
  std::unordered_map<uint64_t, std::promise<buffer_type>> _promises;
  bool _disconnected;

  uint64_t _message_counter;
};

template<typename T> inline T deserialize_from_buffer (buffer_type buffer)
{
  const std::string blob (buffer.begin(), buffer.end());
  std::istringstream is (blob);
  boost::archive::text_iarchive ia (is);
  T value;
  ia & value;
  return value;
}
template<> inline void deserialize_from_buffer (buffer_type buffer)
{
  const std::string blob (buffer.begin(), buffer.end());
  std::istringstream is (blob);
  boost::archive::text_iarchive ia (is);
  if (!is.eof())
  {
    throw std::logic_error ("return value for void function");
  }
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

    const std::function<std::string (std::string)> prepend_size
      ([](std::string str) { return std::to_string (str.size()) + ' ' + str; });

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

#endif
