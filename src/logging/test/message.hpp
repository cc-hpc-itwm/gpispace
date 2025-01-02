// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/testing/random.hpp>

#include <logging/message.hpp>

#include <iosfwd>

namespace fhg
{
  namespace logging
  {
    //! \note operator== and operator<< are not in an anonmyous
    //! namespace for lookup reasons: they are used from a boost
    //! namespace, so only an anonymous namespace in that boost
    //! namespace would be used.
    bool operator== (message const& lhs, message const& rhs);
    std::ostream& operator<< (std::ostream& os, message const& x);
  }

  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<>
          struct random_impl<logging::message, void>
        {
          logging::message operator()() const
          {
            random<std::string> random_string;
            return {random_string(), random_string()};
          }
        };
      }
    }
  }
}
