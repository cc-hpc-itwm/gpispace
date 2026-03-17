// Copyright (C) 2020-2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "gspc/we/type/bytearray.hpp"

//! \note These are not supposed to be used from C, but only need to
//! be un-mangled for symbol resolution in workflow.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
#define SWH_EXPORT __attribute__ ((visibility ("default")))

  SWH_EXPORT std::pair<gspc::we::type::bytearray, bool> stochastic_with_heureka_roll_and_heureka
    ( unsigned long number_of_rolls
    , unsigned long seed
    , gspc::we::type::bytearray user_data
    );

  SWH_EXPORT gspc::we::type::bytearray stochastic_with_heureka_reduce
    ( gspc::we::type::bytearray state
    , gspc::we::type::bytearray partial_result
    , gspc::we::type::bytearray user_data
    );

  SWH_EXPORT gspc::we::type::bytearray stochastic_with_heureka_post_process
    ( unsigned long total_number_of_rolls
    , gspc::we::type::bytearray reduced
    , gspc::we::type::bytearray user_data
    );

#undef SWH_EXPORT
}
#pragma clang diagnostic pop
