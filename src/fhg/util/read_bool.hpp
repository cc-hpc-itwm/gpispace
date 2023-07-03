// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace fhg
{
  namespace util
  {
    //! \note Same as fhg::util::parse::require::boolean, but with ::tolower()
    bool read_bool (std::string const&);
  }
}
