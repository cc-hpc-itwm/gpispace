// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <util-generic/syscall.hpp>

#include <boost/format.hpp>

#include <cstdlib>
#include <exception>

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
        scoped_SIGCHLD_block (scoped_SIGCHLD_block const&) = delete;
        scoped_SIGCHLD_block& operator= (scoped_SIGCHLD_block const&) = delete;
        scoped_SIGCHLD_block (scoped_SIGCHLD_block&&) = delete;
        scoped_SIGCHLD_block& operator= (scoped_SIGCHLD_block&&) = delete;

        sigset_t _signals_to_restore;
      } const signal_blocker;

      if (int ec = std::system (command.c_str()))
      {
        throw std::runtime_error
          (( ::boost::format ("Could not run '%1%': error code '%2%'")
           % command
           % ec
           ).str()
          );
      }
    }
  }
}
