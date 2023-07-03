// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iterator>

namespace fhg
{
  namespace util
  {
    //! \note std::next (it, n) complains when n has not the type
    //! it::difference
    template<typename It, typename D>
      It next (It it, D n)
    {
      std::advance (it, n);
      return it;
    }
  }
}
