// Copyright (C) 2010,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>


  namespace gspc::util
  {
    //! returns true if p is a prefix of x, that is x = pv for some v
    bool starts_with (std::string const& p, std::string const& x);

    //! returns true if s is a suffix of x, that is x = vs for some v
    bool ends_with (std::string const& s, std::string const& x);
  }
