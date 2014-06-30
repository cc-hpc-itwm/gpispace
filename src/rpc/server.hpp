// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_RPC_SERVER_HPP
#define FHG_RPC_SERVER_HPP

#include <rpc/common.hpp>

#include <fhg/util/boost/serialization/tuple.hpp>

#include <network/connection.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/noncopyable.hpp>

#include <functional>
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
      void dispatch
        (network::connection_type* connection, network::buffer_type packet) const;

    private:
      friend struct service_handler;

      std::unordered_map
        < std::string
        , std::function<network::buffer_type (std::string)>
        > _handlers;
    };

    //! \note helper to register service scoped
    struct service_handler : boost::noncopyable
    {
    public:
      service_handler
        ( service_dispatcher& manager
        , std::string name
        , std::function<network::buffer_type (std::string)> handler
        );
      ~service_handler();

    private:
      service_dispatcher& _manager;
      std::string _name;
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

      network::buffer_type operator() (std::string blob)
      {
        result_type ret
          (apply_tuple (_fun, unwrap_arguments<arguments_type> (std::move (blob))));

        std::ostringstream os;
        boost::archive::text_oarchive oa (os);
        oa & ret;
        const std::string os_str (os.str());
        return network::buffer_type (os_str.begin(), os_str.end());
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

      network::buffer_type operator() (std::string blob)
      {
        apply_tuple (_fun, unwrap_arguments<arguments_type> (std::move (blob)));

        std::ostringstream os;
        boost::archive::text_oarchive oa (os);
        const std::string os_str (os.str());
        return network::buffer_type (os_str.begin(), os_str.end());
      }

    private:
      function_type _fun;
    };
  }
}

#endif
