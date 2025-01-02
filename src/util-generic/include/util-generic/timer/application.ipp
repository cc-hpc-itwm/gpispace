// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <utility>

namespace fhg
{
  namespace util
  {
    namespace timer
    {
      template<typename Duration, typename Clock>
        application<Duration, Clock>::application ( std::string description
                                                  , std::ostream& os
                                                  )
          : ostream::echo (os)
          , sections<Duration, Clock> (std::move (description), *this)
      {}
    }
  }
}
