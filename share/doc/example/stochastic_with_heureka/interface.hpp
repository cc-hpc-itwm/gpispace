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
