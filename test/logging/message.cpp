// Copyright (C) 2020,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/logging/message.hpp>

#include <ostream>
#include <tuple>


  namespace gspc::logging
  {
    bool operator== (message const& lhs, message const& rhs)
    {
      return std::tie (lhs._content, lhs._category)
        == std::tie (rhs._content, rhs._category);
    }
    std::ostream& operator<< (std::ostream& os, message const& x)
    {
      return os << "content=" << x._content << ", "
                << "category=" << x._category;
    }
  }
