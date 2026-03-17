#pragma once

#include <gspc/util/mp/none_is.hpp>

#include <type_traits>



    namespace gspc::util::mp
    {
      //! Given a type Needle, checks that types Haystack contains
      //! exactly one Needle, i.e. not more than one or none.
      template<typename Needle, typename... Haystack>
        struct exactly_one_is : std::false_type {};
      template<typename T, typename... Tail>
        struct exactly_one_is<T, T, Tail...> : none_is<T, Tail...> {};
      template<typename T, typename Head, typename... Tail>
        struct exactly_one_is<T, Head, Tail...> : exactly_one_is<T, Tail...> {};
    }
