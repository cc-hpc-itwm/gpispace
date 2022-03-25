// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
