// Copyright (C) 2020-2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/bytearray.hpp>

//! \note workaround for gmp bug: cstddef included with libstdcxx
//! implementation detail that changed in gcc 4.9.0
#include <cstddef>
#include <gmpxx.h>

#include <iosfwd>
#include <string>
#include <utility>

namespace miller_rabin
{
  std::pair<mpz_class, mpz_class> factor_out_powers_of_two (mpz_class const&);

  bool is_witness_for_compositeness (mpz_class const& a, mpz_class const& n);

  gspc::we::type::bytearray generate_user_data (std::string const& number);
  void show_result (std::ostream& os, gspc::we::type::bytearray const& output);
}
