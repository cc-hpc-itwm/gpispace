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

#include <util-generic/interruption_handler.hpp>
#include <util-generic/syscall.hpp>

#include <stdexcept>

#include <sys/signalfd.h>

namespace fhg
{
  namespace util
  {
    void interruption_handler::wait_for_finish_or_interruption
      (std::function<void()> const& on_interruption) const
    {
      signalfd_siginfo fdsi;

      do
      {
        if ( fhg::util::syscall::read (_fd._, &fdsi, sizeof (fdsi))
           != sizeof (fdsi)
           )
        {
          throw std::logic_error ("signalfd read with invalid size");
        }

        if (fdsi.ssi_signo == SIGINT)
        {
          on_interruption();
        }
      } while (fdsi.ssi_signo != SIGUSR2);
    }

    void interruption_handler::finished_on_scope_exit_function()
    {
      fhg::util::syscall::kill (fhg::util::syscall::getpid(), SIGUSR2);
    }
  }
}
