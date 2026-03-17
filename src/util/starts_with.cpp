// Copyright (C) 2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/starts_with.hpp>


  namespace gspc::util
  {
    namespace
    {
      template<typename IT>
        bool generic_starts_with ( IT pos_p, IT end_p
                                 , IT pos_x, IT end_x
                                 )
      {
        while (pos_p != end_p && pos_x != end_x && *pos_p == *pos_x)
        {
          ++pos_p;
          ++pos_x;
        }

        return pos_p == end_p;
      }
    }

    bool starts_with (std::string const& p, std::string const& x)
    {
      return generic_starts_with (p.begin(), p.end(), x.begin(), x.end());
    }

    bool ends_with (std::string const& s, std::string const& x)
    {
      return generic_starts_with (s.rbegin(), s.rend(), x.rbegin(), x.rend());
    }
  }
