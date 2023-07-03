// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/ostream/modifier.hpp>

#include <boost/filesystem.hpp>

#include <iosfwd>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      class include : public ostream::modifier
      {
      public:
        include (std::string const&);
        std::ostream& operator() (std::ostream&) const override;
      private:
        std::string const& _fname;
      };
    }
  }
}
