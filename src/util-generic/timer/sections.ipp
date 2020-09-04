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

#include <util-generic/cxx14/make_unique.hpp>

#include <utility>

namespace fhg
{
  namespace util
  {
    namespace timer
    {
      template<typename Duration, typename Clock>
        sections<Duration, Clock>::sections ( std::string description
                                            , std::ostream& os
                                            )
          : _os (os)
          , _total (std::move (description), _os)
        {}

      template<typename Duration, typename Clock>
        sections<Duration, Clock>::~sections() = default;

      template<typename Duration, typename Clock>
        void sections<Duration, Clock>::end_section()
      {
        _section.reset();
      }

      template<typename Duration, typename Clock>
        void sections<Duration, Clock>::section (std::string description)
      {
        end_section();

        _section = fhg::util::cxx14::make_unique<scoped<Duration, Clock>>
          (std::move (description), _os);
      }
    }
  }
}
