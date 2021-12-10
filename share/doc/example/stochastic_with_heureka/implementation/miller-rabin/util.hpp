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

#include <we/type/bytearray.hpp>

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

  we::type::bytearray generate_user_data (std::string const& number);
  void show_result (std::ostream& os, we::type::bytearray const& output);
}
