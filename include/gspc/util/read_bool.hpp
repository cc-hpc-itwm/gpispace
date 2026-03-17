// Copyright (C) 2010,2012-2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>


  namespace gspc::util
  {
    //! \note Same as gspc::util::parse::require::boolean, but with ::tolower()
    bool read_bool (std::string const&);
  }
