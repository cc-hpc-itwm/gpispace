// Copyright (C) 2018,2020,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/testing/random.hpp>

#include <gspc/logging/message.hpp>

#include <iosfwd>

namespace gspc::logging
{
  //! \note operator== and operator<< are not in an anonmyous
  //! namespace for lookup reasons: they are used from a boost
  //! namespace, so only an anonymous namespace in that boost
  //! namespace would be used.
  bool operator== (message const& lhs, message const& rhs);
  std::ostream& operator<< (std::ostream& os, message const& x);
}

namespace gspc::testing::detail
{
  template<>
    struct random_impl<gspc::logging::message, void>
  {
    gspc::logging::message operator()() const
    {
      gspc::testing::random<std::string> random_string;
      return {random_string(), random_string()};
    }
  };
}
