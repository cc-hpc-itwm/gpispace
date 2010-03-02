/*
 * =====================================================================================
 *
 *       Filename:  types.hpp
 *
 *    Description:  typedefinitions
 *
 *        Version:  1.0
 *        Created:  11/15/2009 04:49:31 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_MODULES_TYPES_HPP
#define SDPA_MODULES_TYPES_HPP 1

#include <list>

#include <sdpa/wf/types.hpp>

namespace sdpa { namespace modules {
  class IModule;

  typedef sdpa::wf::parameters_t data_t;

  typedef void (*InitFunction)(IModule*);
  typedef void (*GenericFunction)(data_t&);

  typedef std::list<std::string> param_names_list_t;
  typedef std::pair<GenericFunction, param_names_list_t> parameterized_function_t;
}}

#endif
