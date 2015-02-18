// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <rpc/client.hpp>
#include <rpc/common.hpp>

#include <rpc/exception_serialization.hpp>

#include <fhg/util/boost/serialization/tuple.hpp>
#include <fhg/util/make_indices.hpp>

#include <network/connection.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/noncopyable.hpp>

#include <functional>
#include <future>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>

namespace fhg
{
  namespace rpc
  {
    struct service_dispatcher
    {
    public:
      service_dispatcher (exception::serialization_functions);

      void dispatch
        (network::connection_type* connection, network::buffer_type packet) const;

    private:
      template<typename> friend struct service_handler;
      template<typename> friend struct aggregated_service_handler;

      std::unordered_map
        < std::string
        , std::function<std::string (std::string)>
        > _handlers;

      exception::serialization_functions _serialization_functions;
      exception::aggregated_serialization_functions
        _aggregated_serialization_functions;
    };

    template<typename> struct thunk;

    //! \note helper to register service scoped
    template<typename> struct service_handler;
    template<typename R, typename... Args>
      struct service_handler<R (Args...)> : boost::noncopyable
    {
    public:
      template<typename Func>
        service_handler
          (service_dispatcher& manager, std::string name, Func handler)
        : _handler_registration
          ( manager._handlers
          , std::move (name)
          , thunk<R (Args...)> (std::move (handler))
          )
      {}

    private:
      util::unique_scoped_map_insert<decltype (service_dispatcher::_handlers)>
        _handler_registration;
    };

    namespace
    {
      //! \note Workaround for gcc bug not allowing variadic capture in lambda.
      template<typename R, typename... Args>
        aggregated_results<R> wrap_or_throw_wrapped
          ( endpoint_type endpoint
          , std::function<R (Args...)> impl
          , Args... args
          )
      {
        try
        {
          return {{endpoint, impl (args...)}};
        }
        catch (...)
        {
          throw aggregated_exception<R> ({{endpoint, std::current_exception()}});
        }
      }
    }

    template<typename> struct aggregated_service_handler;
    template<typename R, typename... Args>
      struct aggregated_service_handler<R (Args...)> : boost::noncopyable
    {
    public:
      template<typename Func>
        aggregated_service_handler
          ( service_dispatcher& manager
          , std::string name
          , Func impl
          //! \todo Shall be determined by rpc, not user
          , std::function<bool (endpoint_type)> is_endpoint
          , std::function<std::list<endpoints_type> (endpoints_type)> split
          , std::function<endpoint_type (endpoints_type const&)> select
          //! \note must be able to handle a request while handling
          //! this request, thus needs to have two threads or has to
          //! be different from the one the io_service used to process
          //! this request as services are not handled deferred
          , std::function<boost::asio::io_service&()> io_service
          )
        : _impl (impl)
        , _aggregated_serialization_functions
          ( manager._aggregated_serialization_functions
          , typeid (R).name()
          , { &exception::aggregated_serialize<R>
            , &exception::aggregated_deserialize<R>
            }
          )
        , _handler_registration
          ( manager._handlers
          , name
          , thunk<aggregated_results<R> (endpoints_type, Args...)>
            ( [this, is_endpoint, split, select, io_service, name, manager]
                (endpoints_type endpoints, Args... args)
              {
                std::list<std::future<aggregated_results<R>>> result_futures;

                endpoints_type::iterator const this_endpoint_it
                  (std::find_if (endpoints.begin(), endpoints.end(), is_endpoint));
                if (this_endpoint_it != endpoints.end())
                {
                  endpoint_type this_endpoint (std::move (*this_endpoint_it));
                  endpoints.erase (this_endpoint_it);
                  result_futures.emplace_back
                    ( std::async ( &wrap_or_throw_wrapped<R, Args...>
                                 , std::move (this_endpoint)
                                 , _impl
                                 , args...
                                 )
                    );
                }

                std::list<remote_endpoint> remote_endpoints;
                std::list<aggregated_remote_function<R (Args...)>> remote_functions;

                for (endpoints_type part : split (endpoints))
                {
                  endpoint_type endpoint (select (part));
                  remote_endpoints.emplace_back
                    ( io_service()
                    , endpoint.address().to_string()
                    , endpoint.port()
                    , manager._serialization_functions
                    );

                  remote_functions.emplace_back (remote_endpoints.back(), name);
                  result_futures.emplace_back
                    (remote_functions.back() (part, args...));
                }

                aggregated_exception<R> return_values;
                for (std::future<aggregated_results<R>>& future : result_futures)
                {
                  try
                  {
                    aggregated_results<R> results (future.get());
                    return_values.succeeded.insert (results.begin(), results.end());
                  }
                  catch (aggregated_exception<R> const& ex)
                  {
                    return_values.succeeded.insert (ex.succeeded.begin(), ex.succeeded.end());
                    return_values.failed.insert (ex.failed.begin(), ex.failed.end());
                  }
                }

                return return_values.failed.empty()
                  ? return_values.succeeded : throw return_values;
              }
            )
          )
      {}

    private:
      std::function<R (Args...)> _impl;
      util::scoped_map_insert<exception::aggregated_serialization_functions>
        _aggregated_serialization_functions;
      util::unique_scoped_map_insert<decltype (service_dispatcher::_handlers)>
        _handler_registration;
    };

    namespace
    {
      template<typename> struct apply_tuple_impl;
      template<std::size_t... Indices>
        struct apply_tuple_impl<fhg::util::indices<Indices...>>
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
        return apply_tuple_impl<fhg::util::make_indices<sizeof... (OpArgs)>>
          ::apply (std::forward<Op> (op), std::forward<std::tuple<OpArgs...>> (t));
      }

      //! \note Alternative to serializing whole tuple

      // template<typename, typename> struct unwrap_arguments_impl;
      // template<typename arguments_type, std::size_t... Indices>
      //   struct unwrap_arguments_impl<arguments_type, fhg::util::indices<Indices...>>
      // {
      //   template<size_t i>
      //     using argument_type = typename std::tuple_element<i, arguments_type>::type;

      //   static arguments_type apply (util::parse::position& buffer)
      //   {
      //     //! \todo Bug in gcc <= 4.9 (?): order not sequential. Needs to
      //     //! be assembled recursively as workaround.
      //     return arguments_type {unwrap<argument_type<Indices>> (buffer)...};
      //   }
      // };

      // template<typename arguments_type>
      //   arguments_type unwrap_arguments (util::parse::position& buffer)
      // {
      //   return unwrap_arguments_impl
      //     < arguments_type
      //     , fhg::util::make_indices<std::tuple_size<arguments_type>::value>
      //     >::apply (buffer);
      // }

      template<typename arguments_type>
        arguments_type unwrap_arguments (std::string blob)
      {
        //! \todo extract istream directly from buffer?
        std::istringstream is (blob);

        boost::archive::text_iarchive ia (is);
        arguments_type args;
        ia & args;

        return args;
      }
    }

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

      std::string operator() (std::string blob)
      {
        result_type ret
          (apply_tuple (_fun, unwrap_arguments<arguments_type> (std::move (blob))));

        std::ostringstream os;
        boost::archive::text_oarchive oa (os);
        oa & ret;

        return os.str();
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

      std::string operator() (std::string blob)
      {
        apply_tuple (_fun, unwrap_arguments<arguments_type> (std::move (blob)));

        std::ostringstream os;
        boost::archive::text_oarchive oa (os);

        return os.str();
      }

    private:
      function_type _fun;
    };
  }
}
