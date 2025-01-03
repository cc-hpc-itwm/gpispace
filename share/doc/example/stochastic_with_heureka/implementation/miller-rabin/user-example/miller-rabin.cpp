// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <miller-rabin/util.hpp>

#include <stochastic_with_heureka/interface.hpp>

#include <string>
#include <utility>

extern "C"
  std::pair<we::type::bytearray, bool> stochastic_with_heureka_roll_and_heureka
    (unsigned long n, unsigned long seed, we::type::bytearray user_data)
{
  std::string const s (user_data.to_string());

  mpz_class N;
  mpz_import (N.get_mpz_t(), s.size(), 1, sizeof (char), 0, 0, s.data());

  struct scoped_miller_rabin_gmp_state
  {
    scoped_miller_rabin_gmp_state (unsigned long seed)
      : _gmp_random_state()
    {
      gmp_randinit_mt (_gmp_random_state);
      gmp_randseed_ui (_gmp_random_state, seed);
    }
    ~scoped_miller_rabin_gmp_state()
    {
      gmp_randclear (_gmp_random_state);
    }

    mpz_class operator() (mpz_class const& N)
    {
      mpz_class a;
      mpz_urandomm
        (a.get_mpz_t(), _gmp_random_state, mpz_class (N - 3).get_mpz_t());

      return a + 2;
    }

  private:
    gmp_randstate_t _gmp_random_state;
  } random_number (seed);

  for (unsigned long roll (0); roll < n; ++roll)
  {
    if (miller_rabin::is_witness_for_compositeness (random_number (N), N))
    {
      return {we::type::bytearray {true}, true};
    }
  }

  return {we::type::bytearray {false}, false};
};

extern "C"
  we::type::bytearray stochastic_with_heureka_reduce
    ( we::type::bytearray partial_resultL_bytearray
    , we::type::bytearray partial_resultR_bytearray
    , we::type::bytearray
    )
{
  bool partial_resultL;
  bool partial_resultR;
  partial_resultL_bytearray.copy (&partial_resultL);
  partial_resultR_bytearray.copy (&partial_resultR);

  return we::type::bytearray {partial_resultL || partial_resultR};
}

extern "C"
  we::type::bytearray stochastic_with_heureka_post_process
    ( unsigned long
    , we::type::bytearray result_bytearray
    , we::type::bytearray
    )
{
  return result_bytearray;
}
