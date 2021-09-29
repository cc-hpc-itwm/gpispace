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

#include <boost/preprocessor/cat.hpp>

#include <utility>

namespace fhg
{
  namespace util
  {
    namespace detail
    {
      template<typename T> struct InThisScope
      {
        struct Construct{};

        InThisScope (Construct, T& variable)
          : _variable (variable)
          , _value (_variable)
        {}
        ~InThisScope()
        {
          _variable = _value;
        }
        InThisScope (InThisScope const&) = delete;
        InThisScope (InThisScope&&) = delete;
        InThisScope& operator= (InThisScope const&) = delete;
        InThisScope& operator= (InThisScope&&) = delete;

        template<typename... Args> void operator() (Args&&... args) const
        {
          _variable = T {std::forward<Args> (args)...};
        }
        template<typename... Args> void operator= (Args&&... args) const
        {
          this->operator() (std::forward<Args> (args)...);
        }
      private:
        T& _variable;
        T _value;
      };
    }
  }
}

#define FHG_UTIL_IN_THIS_SCOPE_IMPL(variable)                               \
  ::fhg::util::detail::InThisScope<decltype (variable)> const               \
    BOOST_PP_CAT (_in_this_scope, __LINE__)                                 \
      ( ::fhg::util::detail::InThisScope<decltype (variable)>::Construct{}  \
      , variable                                                            \
      );                                                                    \
  BOOST_PP_CAT (_in_this_scope, __LINE__)
