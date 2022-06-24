// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
