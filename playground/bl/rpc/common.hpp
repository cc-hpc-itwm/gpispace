// bernd.loerwald@itwm.fraunhofer.de

#ifndef PLAYGROUND_BL_RPC_COMMON_HPP
#define PLAYGROUND_BL_RPC_COMMON_HPP

#include <fhg/util/num.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <cstdint>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

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

#endif
