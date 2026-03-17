// Copyright (C) 2013,2015,2020-2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/parse/error.hpp>

#include <sstream>




      namespace gspc::util::parse::error
      {
        expected::expected (std::string const& what, position const& inp)
          : generic {fmt::format ("expected '{}'", what), inp}
        {}
      }
