// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhg/util/cxx17/void_t.hpp>

#include <type_traits>

namespace fhg
{
  namespace rpc
  {
#define FHG_RPC_FUNCTION_DESCRIPTION(name_, signature_...)      \
    struct name_                                                \
    {                                                           \
      using signature = signature_;                             \
      static constexpr const char* const name = #name_;         \
    }

    template<typename T>
      using is_function_description = util::cxx17::void_t
        < typename T::signature
        , typename std::enable_if<std::is_function<typename T::signature>::value>::type
        , decltype (std::declval<std::string&>() = T::name)
        >;

    template<typename, typename = void>
      struct is_function_description_t : std::false_type
    {};

    template<typename T>
      struct is_function_description_t
        <T, is_function_description<T>> : std::true_type
    {};
  }
}
