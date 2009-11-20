/*
 * =====================================================================================
 *
 *       Filename:  FVMModuleLoader.hpp
 *
 *    Description:  fvm provided module loader
 *
 *        Version:  1.0
 *        Created:  11/06/2009 07:26:24 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_MODULE_FVM_MODULE_LOADER_HPP
#define SDPA_MODULE_FVM_MODULE_LOADER_HPP 1

// we currently don't have a fvm module loader, fall back
#include <sdpa/modules/FallBackModuleLoader.hpp>

namespace sdpa { namespace modules {
  typedef FallBackModuleLoader FVMModuleLoader;
}}

#endif
