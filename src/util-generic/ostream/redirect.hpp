// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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
#include <ostream>
#include <streambuf>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      class redirect : public std::streambuf
      {
      public:
        redirect ( std::ostream& os
                 , std::function<void (std::string const&)> line
                 );

        virtual ~redirect() override;

        virtual int_type overflow (int_type c) override;

      private:
        std::ostream& _os;
        std::function<void (std::string const&)> _line;
        std::string _buffer;
        std::streambuf* _streambuf;
      };

      class prepend_line : public redirect
      {
      public:
        prepend_line ( std::ostream& os
                     , std::function<std::string (std::string const&)> prefix
                     );
        prepend_line ( std::ostream& os
                     , std::function<std::string ()> prefix
                     );
        prepend_line (std::ostream& os, std::string prefix);
      };
    }
  }
}
