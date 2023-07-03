// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/ostream/modifier.hpp>

#include <iosfwd>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace include_guard
      {
        class open : public ostream::modifier
        {
        public:
          open (std::string const&);
          std::ostream& operator() (std::ostream&) const override;
        private:
          const std::string _name;
        };

        class close : public ostream::modifier
        {
        public:
          std::ostream& operator() (std::ostream&) const override;
        };
      }
    }
  }
}
