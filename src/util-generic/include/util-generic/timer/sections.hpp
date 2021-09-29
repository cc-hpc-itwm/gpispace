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

#include <util-generic/timer/scoped.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace timer
    {
      //! Maintains a scoped timer and allows to start and stop
      //! section timers while to overall timer runs. Sections can be
      //! started any time and end the previous section. Running
      //! sections can be stopped any time manually too.

      template<typename Duration, typename Clock = std::chrono::steady_clock>
        struct sections
      {
        sections (std::string, std::ostream& = std::cout);
        ~sections();

        void end_section();
        void section (std::string);

      private:
        std::ostream& _os;
        scoped<Duration, Clock> const _total;
        std::unique_ptr<scoped<Duration, Clock>> _section;
      };
    }
  }
}

#include <util-generic/timer/sections.ipp>
