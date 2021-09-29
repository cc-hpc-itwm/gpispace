// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <util-generic/functor_visitor.hpp>
#include <util-generic/mp/exactly_one_is.hpp>

#include <boost/variant.hpp>

#include <string>
#include <type_traits>
#include <typeinfo>

namespace fhg
{
  namespace util
  {
    template<typename From, typename To, typename Element>
      bad_variant_cast bad_variant_cast::make()
    {
      return bad_variant_cast
        (typeid (From).name(), typeid (To).name(), typeid (Element).name());
    }
    bad_variant_cast::bad_variant_cast (std::string f, std::string t, std::string e)
      : std::logic_error ( "Unable to convert From=" + f + " to To=" + t
                         + ": " + e + " not contained in To"
                         )
    {}


    namespace detail
    {
      template<typename From, typename... To>
        struct variant_cast_visitor
          : boost::static_visitor<boost::variant<To...>>
      {
        variant_cast_visitor () = default;

        using target = boost::variant<To...>;
        template<typename T>
          using contained
            = mp::exactly_one_is<typename std::decay<T>::type, To...>;
        template<bool b> using iff = typename std::enable_if<b>::type*;

        template<typename T>
          target operator() (T&& x, iff<contained<T>{}> = nullptr) const
        {
          return x;
        }
        template<typename T>
          target operator() (T&&, iff<!contained<T>{}> = nullptr) const
        {
          throw bad_variant_cast::make<From, boost::variant<To...>, T>();
        }
      };

      template<typename To>
        struct variant_cast_impl;
      template<typename... To>
        struct variant_cast_impl<boost::variant<To...>>
      {
        template<typename From>
          boost::variant<To...> operator() (From&& from) const
        {
          return visit<boost::variant<To...>>
            (std::forward<From> (from), variant_cast_visitor<From, To...>{});
        }
      };
    }

    template<typename To, typename From>
      To variant_cast (From&& from)
    {
      return detail::variant_cast_impl<To>{} (std::forward<From> (from));
    }

    namespace detail
    {
      //! Container<?>=From -> Container<To>
      template<typename From, typename To>
        struct variants_cast_return_type_impl;

      template<typename Alloc, typename T>
        using rebind
          = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;

      template < template<class, class> class Container
               , typename Elem
               , typename To
               , typename Allocator
               >
        struct variants_cast_return_type_impl<Container<Elem, Allocator>, To>
      {
        using type = Container<To, rebind<Allocator, To>>;
      };

      template<typename From, typename To>
        struct variants_cast_return_type
          : variants_cast_return_type_impl<typename std::decay<From>::type, To>
      {
      };
    }

    template<typename To, typename From>
      typename detail::variants_cast_return_type<From, To>::type variants_cast
        (From&& from)
    {
      typename detail::variants_cast_return_type<From, To>::type result;
      for (auto&& f : from)
      {
        result.emplace_back (variant_cast<To> (std::move (f)));
      }
      return result;
    }
  }
}
