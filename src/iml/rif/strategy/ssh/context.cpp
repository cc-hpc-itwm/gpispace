#include <iml/rif/strategy/ssh/context.hpp>

#include <iml/rif/strategy/ssh/detail.hpp>

#include <util-generic/syscall.hpp>

#include <atomic>
#include <exception>
#include <stdexcept>

#include <openssl/crypto.h>
#include <pthread.h>

namespace
{
  //! \todo avoid the GLOBAL and deliver a pointer to a member function
  static pthread_mutex_t* GLOBAL_crypto_locks;
  static std::atomic<bool> GLOBAL_ssh_context_initialized (false);

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
      (OPENSSL_malloc (CRYPTO_num_locks() * sizeof (pthread_mutex_t)));

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
    CRYPTO_set_locking_callback (&locking_callback);
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