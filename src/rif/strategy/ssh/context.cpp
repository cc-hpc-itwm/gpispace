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

#include <rif/strategy/ssh/context.hpp>

#include <rif/strategy/ssh/detail.hpp>

#include <atomic>
#include <exception>
#include <stdexcept>

#include <pthread.h>

namespace
{
  //! \todo avoid the GLOBAL and deliver a pointer to a member function
  static std::atomic<bool> GLOBAL_ssh_context_initialized (false);
}

namespace libssh2
{
  context::context()
  try
  {
    if (std::atomic_exchange (&GLOBAL_ssh_context_initialized, true))
    {
      throw std::logic_error ("libssh2::context must be a singleton");
    }

    detail::wrapped::init (0);
  }
  catch (...)
  {
    std::throw_with_nested (std::runtime_error ("initializing libssh2"));
  }

  context::~context()
  {
    detail::wrapped::exit();

    std::atomic_exchange (&GLOBAL_ssh_context_initialized, false);
  }
}
