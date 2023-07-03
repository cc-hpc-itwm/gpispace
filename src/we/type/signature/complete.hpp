// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iosfwd>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      class complete
      {
      public:
        complete (std::string const&);
        std::string const& tname() const;
      private:
        std::string const& _tname;
      };
      std::ostream& operator<< (std::ostream&, complete const&);
    }
  }
}
