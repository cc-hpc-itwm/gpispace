// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
