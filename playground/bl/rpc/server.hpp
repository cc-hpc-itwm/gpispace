// bernd.loerwald@itwm.fraunhofer.de

#ifndef PLAYGROUND_BL_RPC_SERVER_HPP
#define PLAYGROUND_BL_RPC_SERVER_HPP

#include <playground/bl/rpc/common.hpp>

#include <fhg/util/parse/position.hpp>

#include <playground/bl/net/connection.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/noncopyable.hpp>

#include <functional>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>

struct service_dispatcher
{
public:
  void dispatch (connection_type* connection, buffer_type packet) const;

private:
  friend struct service_handler;

  std::unordered_map
    < std::string
    , std::function<buffer_type (fhg::util::parse::position&)>
    > _handlers;
};

//! \note helper to register service scoped
struct service_handler : boost::noncopyable
{
public:
  service_handler ( service_dispatcher& manager
                  , std::string name
                  , std::function<buffer_type (fhg::util::parse::position&)> handler
                  );
  ~service_handler();

private:
  service_dispatcher& _manager;
  std::string _name;
};

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

#endif
