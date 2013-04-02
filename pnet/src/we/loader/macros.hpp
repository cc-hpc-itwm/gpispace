/*
 * =====================================================================================
 *
 *       Filename:  Macros.hpp
 *
 *    Description:  just the macro definitions for the modules
 *
 *        Version:  1.0
 *        Created:  11/14/2009 09:58:06 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_LOADER_MACROS_HPP
#define WE_LOADER_MACROS_HPP 1

#include <we/loader/IModule.hpp>

#define WE_MOD_INITIALIZE_START(modname)\
  extern "C"\
  {\
     void __attribute__ ((visibility ("default")))                    \
          we_mod_initialize(::we::loader::IModule *, unsigned int);   \
     void we_mod_initialize(::we::loader::IModule *mod, unsigned int)                \
     {\
        (void)(mod);                               \
        {volatile int _ = WE_GUARD_SYMBOL; (void)_;} \
        mod->name (#modname)

#define WE_REGISTER_FUN_START(fun)                \
  {                                               \
    ::we::loader::param_names_list_t params

#define WE_ADD_INP(p, typ)\
           params.push_back(p)
#define WE_ADD_OUT(p, typ)\
           params.push_back(p)

#define WE_REGISTER_FUN_END(fun)\
           mod->add_function(#fun, &fun, params);\
        }

#define WE_REGISTER_FUN(fun)                   \
        mod->add_function(#fun, &fun)
#define WE_REGISTER_FUN_AS(fun,as)             \
        mod->add_function(as, &fun)

#define WE_SET_STATE(s)\
        mod->state (s)

#define WE_MOD_INITIALIZE_END(modname)\
    }\
  }

#define WE_MOD_FINALIZE_START(modname)\
  extern "C"\
  {\
     void __attribute__ ((visibility ("default")))   \
          we_mod_finalize(::we::loader::IModule *);  \
     void we_mod_finalize(::we::loader::IModule *mod)\
     {\
        (void)(mod)

#define WE_GET_STATE()\
         mod->state()

#define WE_CLEAR_STATE()\
         mod->state(0)

#define WE_MOD_FINALIZE_END(modname)\
     }\
  }

#endif
