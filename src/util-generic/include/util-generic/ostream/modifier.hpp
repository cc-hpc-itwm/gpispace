// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iosfwd>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      class modifier
      {
      public:
        virtual std::ostream& operator() (std::ostream&) const = 0;
        virtual ~modifier() = default;
        modifier() = default;
        modifier (modifier const&) = default;
        modifier (modifier&&) = default;
        modifier& operator= (modifier const&) = default;
        modifier& operator= (modifier&&) = default;

        std::string string() const;
      };
      std::ostream& operator<< (std::ostream&, const modifier&);
    }
  }
}
