// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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
        //! \note explicitly define, as definition of implicit copy
        //! constructor for 'modifier' is deprecated because it has a
        //! user-declared destructor [-Werror,-Wdeprecated]
        modifier (modifier const&) = default;
        modifier& operator= (modifier const&) = default;

        std::string string() const;
      };
      std::ostream& operator<< (std::ostream&, const modifier&);
    }
  }
}
