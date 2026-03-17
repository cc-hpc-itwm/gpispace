// Copyright (C) 2012-2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/read_bool.hpp>

#include <gspc/util/parse/position.hpp>
#include <gspc/util/parse/require.hpp>

#include <algorithm>
#include <cctype>
#include <iterator>
#include <stdexcept>


  namespace gspc::util
  {
    bool read_bool (std::string const& _inp)
    {
      std::string inp;

      std::transform ( _inp.begin(), _inp.end()
                     , std::back_inserter (inp)
                     , [] (unsigned char c) { return std::tolower (c); }
                     );

      parse::position pos (inp);

      return parse::require::boolean (pos);
    }
  }
