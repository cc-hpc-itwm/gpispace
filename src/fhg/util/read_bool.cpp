// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/read_bool.hpp>

#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <algorithm>
#include <cctype>
#include <iterator>
#include <stdexcept>

namespace fhg
{
  namespace util
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
}
