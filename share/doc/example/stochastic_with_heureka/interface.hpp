// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "we/type/bytearray.hpp"

//! \note These are not supposed to be used from C, but only need to
//! be un-mangled for symbol resolution in workflow.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
#define SWH_DLLEXPORT __attribute__ ((visibility ("default")))

  SWH_DLLEXPORT std::pair<we::type::bytearray, bool> stochastic_with_heureka_roll_and_heureka
    ( unsigned long number_of_rolls
    , unsigned long seed
    , we::type::bytearray user_data
    );

  SWH_DLLEXPORT we::type::bytearray stochastic_with_heureka_reduce
    ( we::type::bytearray state
    , we::type::bytearray partial_result
    , we::type::bytearray user_data
    );

  SWH_DLLEXPORT we::type::bytearray stochastic_with_heureka_post_process
    ( unsigned long total_number_of_rolls
    , we::type::bytearray reduced
    , we::type::bytearray user_data
    );

#undef SWH_DLLEXPORT
}
#pragma clang diagnostic pop
