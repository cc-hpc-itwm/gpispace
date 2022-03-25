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

#include <fhg/util/indenter.hpp>
#include <util-generic/ostream/modifier.hpp>

#include <boost/optional.hpp>

#include <string>
#include <iosfwd>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace structure
      {
        class open : public ostream::modifier
        {
        public:
          open (fhg::util::indenter&);
          open (fhg::util::indenter&, std::string const&);
          virtual std::ostream& operator() (std::ostream&) const override;

        private:
          fhg::util::indenter& _indent;
          const ::boost::optional<std::string> _tag;
        };

        class close : public ostream::modifier
        {
        public:
          close (fhg::util::indenter&);
          virtual std::ostream& operator() (std::ostream&) const override;

        private:
          fhg::util::indenter& _indent;
        };
      }
    }
  }
}
