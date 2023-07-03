// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <iostream>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace timer
    {
      //! Print START and DONE of a scope including the description and
      //! the execution time with given resolution.

      template<typename Duration, typename Clock = std::chrono::steady_clock>
        struct scoped
      {
        scoped (std::string, std::ostream& = std::cout);
        ~scoped();

        scoped (scoped const&) = delete;
        scoped& operator= (scoped const&) = delete;
        scoped (scoped&&) = delete;
        scoped& operator= (scoped&&) = delete;

      private:
        std::ostream& _os;
        std::string const _description;
        typename Clock::time_point const _start;
      };
    }
  }
}

#include <util-generic/timer/scoped.ipp>
