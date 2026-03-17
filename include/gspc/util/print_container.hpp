#pragma once

#include <gspc/util/join.hpp>


  namespace gspc::util
  {
    template<typename C>
      constexpr join_reference<C, std::string>
        print_container
          ( const std::string& open
          , const std::string& separator
          , const std::string& close
          , C const& c
          , ostream::callback::print_function<Value<C>> const& print
          = ostream::callback::id<Value<C>>()
          , std::optional<typename std::iterator_traits<typename std::remove_reference<C>::type::const_iterator>::difference_type>
              const& max_elements_to_print = std::nullopt
          )
    {
      return join_reference<C, std::string>
        (c, separator, print, open, close, max_elements_to_print);
    }
  }
