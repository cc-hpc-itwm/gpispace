// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <implementation/miller-rabin/util.hpp>

#include <utility>

BOOST_AUTO_TEST_CASE (factor_out_powers_of_two)
{
  {
    std::pair<mpz_class, mpz_class> const sm
      (miller_rabin::factor_out_powers_of_two ((1 << 10) * 13 + 1));

    BOOST_CHECK_EQUAL (sm.first, mpz_class (10));
    BOOST_CHECK_EQUAL (sm.second, mpz_class (13));
  }

  {
    std::pair<mpz_class, mpz_class> const sm
      ( miller_rabin::factor_out_powers_of_two
        (mpz_class ("91270843213599097018040811446599681"))
      );

    BOOST_CHECK_EQUAL (sm.first, mpz_class (31));
    BOOST_CHECK_EQUAL (sm.second, mpz_class ("42501298344507392969932785"));
  }

  {
    std::pair<mpz_class, mpz_class> const sm
      ( miller_rabin::factor_out_powers_of_two
        (mpz_class ("845100400152152934331135470251"))
      );

    BOOST_CHECK_EQUAL (sm.first, mpz_class (1));
    BOOST_CHECK_EQUAL (sm.second, mpz_class ("422550200076076467165567735125"));
  }
}

BOOST_AUTO_TEST_CASE (is_witness_for_compositeness)
{
  BOOST_CHECK (miller_rabin::is_witness_for_compositeness (137, 221));
  BOOST_CHECK (!miller_rabin::is_witness_for_compositeness (174, 221));

  BOOST_CHECK
    ( !miller_rabin::is_witness_for_compositeness
      (mpz_class {"135038038286232780753002795959"}, mpz_class {"845100400152152934331135470251"})
    );

  BOOST_CHECK
    ( !miller_rabin::is_witness_for_compositeness
      (mpz_class {"156424771541422532123012642736"}, mpz_class {"845100400152152934331135470251"})
    );
}
