// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      class position
      {
      public:
        position (std::string const&);

        char operator*() const;
        void operator++();
        bool end() const;
        std::size_t eaten() const;

        std::string error_message (std::string const&) const;

      private:
        std::size_t _k {0};
        std::string::const_iterator _pos;
        const std::string::const_iterator _begin;
        const std::string::const_iterator _end;
      };
    }
  }
}
