// Copyright (C) 2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/cpp/include.hpp>



    namespace gspc::util::cpp
    {
      include::include (std::string const& fname)
        : _fname (fname)
      {}
      std::ostream& include::operator() (std::ostream& os) const
      {
        return os << "#include <" << _fname << ">" << std::endl;
      }
    }
