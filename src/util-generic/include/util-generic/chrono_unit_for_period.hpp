// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace fhg
{
  namespace util
  {
    //! Determine a human readable string representing the unit for
    //! the given \c std::chrono::duration::period. SI units are
    //! represented by their standard symbols. Otherwise, the period
    //! is represented as "[n]s" or "[n/d]s" if d != 1.
    template<typename Period>
      std::string chrono_unit_for_period();
  }
}

#include <util-generic/chrono_unit_for_period.ipp>
