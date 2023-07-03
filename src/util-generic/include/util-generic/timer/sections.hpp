// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

        sections (sections const&) = delete;
        sections& operator= (sections const&) = delete;
        sections (sections&&) = delete;
        sections& operator= (sections&&) = delete;

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
