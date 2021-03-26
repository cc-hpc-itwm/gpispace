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

#include <iml/rif/strategy/ssh/context.hpp>

#include <iml/rif/strategy/ssh/detail.hpp>

#include <util-generic/syscall.hpp>

#include <atomic>
#include <exception>
#include <stdexcept>

#include <openssl/crypto.h>
#include <pthread.h>

// The macro function below uses the same formula as the OPENSSL_VERSION_NUMBER does for
// convenience
#define VERSION_NUMBER(MAJOR, MINOR, PATCH, TWEAK) \
  ( (MAJOR << 28) \
  | (MINOR << 20) \
  | (PATCH << 4) \
  | TWEAK \
  )

// OPENSSL_VERSION_NUMBER is marked as deprecated for OpenSSL >1.1.1, but offering separate
// definitions for the major, minor, and patch version numbers.
#ifndef OPENSSL_VERSION_NUMBER
# ifdef OPENSSL_VERSION_PRE_RELEASE
#   define _PRE_RELEASE 0x0L
# else
#   define _PRE_RELEASE 0xfL
# endif
# define OPENSSL_VERSION_NUMBER \
  VERSION_NUMBER \
    ( OPENSSL_VERSION_MAJOR \
    , OPENSSL_VERSION_MINOR \
    , OPENSSL_VERSION_PATCH \
    , _PRE_RELEASE
    )
#endif


namespace
{
  //! \todo avoid the GLOBAL and deliver a pointer to a member function
  static pthread_mutex_t* GLOBAL_crypto_locks;
  static std::atomic<bool> GLOBAL_ssh_context_initialized (false);

#if OPENSSL_VERSION_NUMBER < VERSION_NUMBER(1, 1, 0, 0)
  void locking_callback
    (int mode, int type, char const* /*file*/, int /*line*/)
  {
    if (mode & CRYPTO_LOCK)
    {
      fhg::util::syscall::pthread_mutex_lock (GLOBAL_crypto_locks + type);
    }
    else
    {
      fhg::util::syscall::pthread_mutex_unlock (GLOBAL_crypto_locks + type);
    }
  }
#endif
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

    GLOBAL_crypto_locks = static_cast<pthread_mutex_t*>
      //! \todo update to openssl-1.1.1 to avoid old style cast in macro definition
      //! (OPENSSL_malloc (CRYPTO_num_locks() * sizeof (pthread_mutex_t)));
          (CRYPTO_malloc  (CRYPTO_num_locks() * sizeof (pthread_mutex_t), __FILE__, __LINE__));

    try
    {
      for (int i (0); i < CRYPTO_num_locks(); ++i)
      {
        fhg::util::syscall::pthread_mutex_init
          (GLOBAL_crypto_locks + i, nullptr);
      }
    }
    catch (...)
    {
      OPENSSL_free (GLOBAL_crypto_locks);

      throw;
    }

    //\todo deprecated
    //\todo check return values
    CRYPTO_set_id_callback (&pthread_self);
#if OPENSSL_VERSION_NUMBER < VERSION_NUMBER(1, 1, 0, 0)
    CRYPTO_set_locking_callback (&locking_callback);
#endif
  }
  catch (...)
  {
    std::throw_with_nested (std::runtime_error ("initializing libssh2"));
  }

  context::~context()
  {
    CRYPTO_set_locking_callback (nullptr);

    for (int i (0); i < CRYPTO_num_locks(); ++i)
    {
      fhg::util::syscall::pthread_mutex_destroy (GLOBAL_crypto_locks + i);
    }

    OPENSSL_free (GLOBAL_crypto_locks);

    detail::wrapped::exit();

    std::atomic_exchange (&GLOBAL_ssh_context_initialized, false);
  }
}
