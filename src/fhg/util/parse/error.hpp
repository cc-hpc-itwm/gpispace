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

#include <stdexcept>

#include <fhg/util/parse/position.hpp>

#include <boost/format.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      namespace error
      {
        class generic : public std::runtime_error
        {
        public:
          generic (std::string const& msg, position const& inp)
            : std::runtime_error (inp.error_message (msg))
          {}
          generic (::boost::format const& msg, position const& inp)
            : std::runtime_error (inp.error_message (msg.str()))
          {}
        };

        class expected : public generic
        {
        public:
          expected (std::string const&, position const&);
        };

        template<typename From, typename To>
          class value_too_big : public generic
        {
        public:
          value_too_big (From const& f, position const& pos)
            : generic ( ::boost::format ("value %1% larger than maximum %2%")
                      % f
                      % std::numeric_limits<To>::max()
                      , pos
                      )
          {}
        };

        template<typename I>
          class unexpected_digit : public generic
        {
        public:
          unexpected_digit (position const& pos)
            : generic ( ::boost::format
                        ( "unexpected digit"
                        " (parsed value would be larger than %1%)"
                        )
                      % std::numeric_limits<I>::max()
                      , pos
                      )
          {}
        };
      }
    }
  }
}
