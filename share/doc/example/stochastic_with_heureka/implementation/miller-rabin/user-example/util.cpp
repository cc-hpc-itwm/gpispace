// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <miller-rabin/util.hpp>

namespace miller_rabin
{
  // factor_out_powers_of_two :: Integer -> (Integer, Integer)
  // factor_out_powers_of_two n = div2 0 (n - 1)
  //   where div2 s m = let (q,r) = divMod m 2
  //                    in case r of 0 -> div2 (s + 1) q
  //                                 1 -> (s, m)

  std::pair<mpz_class, mpz_class> factor_out_powers_of_two
    (mpz_class const& n)
  {
    mpz_class s (0);
    mpz_class m (n - 1);

  STEP:
    mpz_class q (m / 2);
    mpz_class r (m % 2);

    if (r == 0)
    {
      ++s;

      m = q;

      goto STEP;
    }

    return {s, m};
  }

  bool is_witness_for_compositeness (mpz_class const& a, mpz_class const& n)
  {
    std::pair<mpz_class, mpz_class> sm (factor_out_powers_of_two (n));
    mpz_class x;
    mpz_powm
      (x.get_mpz_t(), a.get_mpz_t(), sm.second.get_mpz_t(), n.get_mpz_t());

    if ((x == 1) || (x == n - 1))
    {
      return false;
    }

    while (sm.first --> 0)
    {
      x = ((x * x) % n);

      if (x == 1)
      {
        return true;
      }

      if (x == n - 1)
      {
        return false;
      }
    }

    return true;
  }

  we::type::bytearray generate_user_data (std::string const& number)
  {
    std::size_t count;
    void* buffer
      ( mpz_export
        (NULL, &count, 1, sizeof (char), 0, 0, mpz_class (number).get_mpz_t())
      );
    return we::type::bytearray
      (std::string (static_cast<char*> (buffer), count));
  }

  void show_result (std::ostream& os, we::type::bytearray const& output)
  {
    bool result;
    output.copy (&result);

    os << "result = " << (result ? "composite" : "probably prime") << "\n";
  }
}
