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

#include <gspc/detail/dllexport.hpp>

#include <we/loader/IModule.hpp>

#define WE_MOD_INITIALIZE_START()\
  extern "C" \
  {\
     void GSPC_DLLEXPORT we_mod_initialize(::we::loader::IModule *);    \
     void GSPC_DLLEXPORT we_mod_initialize(::we::loader::IModule *mod)  \
     {\
        (void)(mod);                               \
        {volatile unsigned int _ = WE_GUARD_SYMBOL; (void)_;}

#define WE_REGISTER_FUN_AS(fun,as)             \
        mod->add_function(as, &fun)

#define WE_MOD_INITIALIZE_END()\
    }\
  }
