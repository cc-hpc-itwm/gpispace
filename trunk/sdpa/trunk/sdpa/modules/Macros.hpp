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

#ifndef SDPA_MODULES_MACROS_HPP
#define SDPA_MODULES_MACROS_HPP 1

#include <sdpa/modules/Module.hpp>

#define SDPA_MOD_INIT_START(modname)\
  extern "C"\
  {\
    void sdpa_mod_init(Module *mod)\
    {\
      mod->name(#modname);

#define SDPA_REGISTER_FUN_START(fun)\
      ::sdpa::modules::Module::names_list_t params

#define SDPA_ADD_INP(p, typ) params.push_back(p)
#define SDPA_ADD_OUT(p, typ) params.push_back(p)

#define SDPA_REGISTER_FUN_END(fun)\
      mod->add_function(#fun, &fun, params)

#define SDPA_REGISTER_FUN(fun)\
      mod->add_function(#fun, &fun)

#define SDPA_MOD_INIT_END(modname)\
    }\
  }

#endif
