// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_RPC_SERVER_HPP
#define FHG_RPC_SERVER_HPP

#include <rpc/client.hpp>
#include <rpc/common.hpp>

#include <rpc/exception_serialization.hpp>

#include <fhg/util/boost/serialization/tuple.hpp>

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

      exception::serialization_functions _from_exception_ptr_functions;
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
        , _aggregated_serialization_function
          ( manager._aggregated_serialization_functions
          , typeid (R).name()
          , &exception::aggregated_serialize<R>
          )
        , _handler_registration
          ( manager._handlers
          , name
          , thunk<aggregated_results<R> (endpoints_type, Args...)>
            ( [this, is_endpoint, split, select, io_service, name]
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
                    ( std::async
                    ( [this, this_endpoint, &args..., name]() -> aggregated_results<R>
                      {
                        try
                        {
                          return {{this_endpoint, _impl (args...)}};
                        }
                        catch (...)
                        {
                          throw aggregated_exception<R>
                            ({{this_endpoint, std::current_exception()}});
                        }
                      }
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
                    , exception::deserialization_functions()
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
        _aggregated_serialization_function;
      util::unique_scoped_map_insert<decltype (service_dispatcher::_handlers)>
        _handler_registration;
    };

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
      //     <arguments_type, make_indices<std::tuple_size<arguments_type>::value>>
      //     ::apply (buffer);
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

#endif
