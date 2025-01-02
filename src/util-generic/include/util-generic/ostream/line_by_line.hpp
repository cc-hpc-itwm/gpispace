// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <streambuf>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      class line_by_line : public std::streambuf
      {
      public:
        line_by_line (std::function<void (std::string const&)> const& callback);
        ~line_by_line() override;

        int_type overflow (int_type i) override;

        line_by_line (line_by_line const&) = delete;
        line_by_line (line_by_line&&) = delete;
        line_by_line& operator= (line_by_line const&) = delete;
        line_by_line& operator= (line_by_line&&) = delete;

      private:
        std::string _buffer;
        std::function<void (std::string const&)> _callback;
      };
    }
  }
}
