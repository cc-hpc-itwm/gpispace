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

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <util-generic/syscall.hpp>

#include <boost/format.hpp>

#include <exception>
#include <cstdlib>

namespace fhg
{
  namespace util
  {
    void system_with_blocked_SIGCHLD (std::string const& command)
    {
      struct scoped_SIGCHLD_block
      {
        scoped_SIGCHLD_block()
        {
          sigset_t signals_to_block;
          util::syscall::sigemptyset (&signals_to_block);
          util::syscall::sigaddset (&signals_to_block, SIGCHLD);
          util::syscall::pthread_sigmask
            (SIG_BLOCK, &signals_to_block, &_signals_to_restore);
        }
        ~scoped_SIGCHLD_block()
        {
          util::syscall::pthread_sigmask
            (SIG_UNBLOCK, &_signals_to_restore, nullptr);
        }
        sigset_t _signals_to_restore;
      } const signal_blocker;

      if (int ec = std::system (command.c_str()))
      {
        throw std::runtime_error
          (( boost::format ("Could not run '%1%': error code '%2%'")
           % command
           % ec
           ).str()
          );
      }
    }
  }
}
