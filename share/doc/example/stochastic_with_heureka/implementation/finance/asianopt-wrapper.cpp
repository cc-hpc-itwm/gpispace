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

#include <implementation/finance/asianopt.hpp>

#include <interface.hpp>

#include <utility>

namespace
{
  template<typename T> T to (we::type::bytearray const& bytearray)
  {
    T t;
    bytearray.copy (&t);
    return t;
  }
}

extern "C"
{
  std::pair<we::type::bytearray, bool>
    stochastic_with_heureka_roll_and_heureka ( unsigned long number_of_rolls
                                             , unsigned long seed
                                             , we::type::bytearray user_data
                                             )
  {
    return { we::type::bytearray
               ( asianopt::roll ( to<asianopt::Parameters> (user_data)
                                , number_of_rolls
                                , seed
                                )
               )
           , false
           };
  }

  we::type::bytearray stochastic_with_heureka_reduce
    ( we::type::bytearray state
    , we::type::bytearray partial_result
    , we::type::bytearray
    )
  {
    return we::type::bytearray
      ( asianopt::reduce ( to<asianopt::Reduced> (state)
                         , to<asianopt::RollResult> (partial_result)
                         )
      );
  }

  we::type::bytearray stochastic_with_heureka_post_process
    ( unsigned long number_of_rolls
    , we::type::bytearray reduced
    , we::type::bytearray user_data
    )
  {
    return we::type::bytearray
      ( asianopt::post_process ( number_of_rolls
                               , to<asianopt::Reduced> (reduced)
                               , to<asianopt::Parameters> (user_data)
                               )
      );
  }
}
