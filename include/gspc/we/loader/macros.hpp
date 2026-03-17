// Copyright (C) 2010,2013,2015-2016,2021,2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/loader/IModule.hpp>

#define WE_MOD_INITIALIZE_START()\
  extern "C" \
  {\
     void GSPC_EXPORT we_mod_initialize(::gspc::we::loader::IModule *);    \
     void GSPC_EXPORT we_mod_initialize(::gspc::we::loader::IModule *mod)  \
     {\
        (void)(mod);                               \
        {volatile unsigned int _ = WE_GUARD_SYMBOL; (void)_;}

#define WE_REGISTER_FUN_AS(fun,as)             \
        mod->add_function(as, &fun)

#define WE_MOD_INITIALIZE_END()\
    }\
  }
