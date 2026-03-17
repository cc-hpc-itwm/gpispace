// Copyright (C) 2014-2015,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/system_with_blocked_SIGCHLD.hpp>

#include <gspc/util/syscall.hpp>

#include <fmt/core.h>
#include <cstdlib>
#include <exception>


  namespace gspc::util
  {
    void system_with_blocked_SIGCHLD (std::string const& command)
    {
      struct scoped_SIGCHLD_block
      {
        scoped_SIGCHLD_block()
        {
          sigset_t signals_to_block;
          gspc::util::syscall::sigemptyset (&signals_to_block);
          gspc::util::syscall::sigaddset (&signals_to_block, SIGCHLD);
          gspc::util::syscall::pthread_sigmask
            (SIG_BLOCK, &signals_to_block, &_signals_to_restore);
        }
        ~scoped_SIGCHLD_block()
        {
          gspc::util::syscall::pthread_sigmask
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
          { fmt::format
              ( "Could not run '{}': error code '{}'"
              , command
              , ec
              )
          };
      }
    }
  }
