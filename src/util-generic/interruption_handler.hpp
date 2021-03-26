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

#include <functional>

#include <util-generic/finally.hpp>
#include <util-generic/syscall/process_signal_block.hpp>
#include <util-generic/syscall/signal_fd.hpp>
#include <util-generic/syscall/signal_set.hpp>

namespace fhg
{
  namespace util
  {
    //! \note construct before starting ANY thread, e.g. as the very
    //! first command in main(), or else a random thread may receive
    //! the signal
    class interruption_handler
    {
    public:
      interruption_handler()
        : _mask ({SIGINT, SIGUSR2})
        , _block (_mask)
        , _fd (_mask)
      {}

      void wait_for_finish_or_interruption
        (std::function<void()> const& on_interruption) const;

    private:
      static void finished_on_scope_exit_function();

    public:
      decltype (finally (&interruption_handler::finished_on_scope_exit_function))
        notifier() const
      {
        return finally (&interruption_handler::finished_on_scope_exit_function);
      }

    private:
      fhg::util::syscall::signal_set _mask;
      fhg::util::syscall::process_signal_block _block;
      fhg::util::syscall::signal_fd _fd;
    };
  }
}
