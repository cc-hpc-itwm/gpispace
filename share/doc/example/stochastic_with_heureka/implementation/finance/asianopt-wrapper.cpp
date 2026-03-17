// Copyright (C) 2020-2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <implementation/finance/asianopt.hpp>

#include <interface.hpp>

#include <utility>

namespace
{
  template<typename T> T to (gspc::we::type::bytearray const& bytearray)
  {
    T t;
    bytearray.copy (&t);
    return t;
  }
}

extern "C"
{
  std::pair<gspc::we::type::bytearray, bool>
    stochastic_with_heureka_roll_and_heureka ( unsigned long number_of_rolls
                                             , unsigned long seed
                                             , gspc::we::type::bytearray user_data
                                             )
  {
    return { gspc::we::type::bytearray
               ( asianopt::roll ( to<asianopt::Parameters> (user_data)
                                , number_of_rolls
                                , seed
                                )
               )
           , false
           };
  }

  gspc::we::type::bytearray stochastic_with_heureka_reduce
    ( gspc::we::type::bytearray state
    , gspc::we::type::bytearray partial_result
    , gspc::we::type::bytearray
    )
  {
    return gspc::we::type::bytearray
      ( asianopt::reduce ( to<asianopt::Reduced> (state)
                         , to<asianopt::RollResult> (partial_result)
                         )
      );
  }

  gspc::we::type::bytearray stochastic_with_heureka_post_process
    ( unsigned long number_of_rolls
    , gspc::we::type::bytearray reduced
    , gspc::we::type::bytearray user_data
    )
  {
    return gspc::we::type::bytearray
      ( asianopt::post_process ( number_of_rolls
                               , to<asianopt::Reduced> (reduced)
                               , to<asianopt::Parameters> (user_data)
                               )
      );
  }
}
