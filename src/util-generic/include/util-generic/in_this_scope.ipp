// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
