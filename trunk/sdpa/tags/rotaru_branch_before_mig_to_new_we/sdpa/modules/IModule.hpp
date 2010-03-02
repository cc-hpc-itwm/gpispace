/*
 * =====================================================================================
 *
 *       Filename:  IModule.hpp
 *
 *    Description:  the module interface
 *
 *        Version:  1.0
 *        Created:  11/15/2009 04:48:38 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_MODULES_IMODULE_HPP
#define SDPA_MODULES_IMODULE_HPP 1

#include <string>

#include <sdpa/modules/types.hpp>
#include <sdpa/modules/exceptions.hpp>

#include <fhglog/fhglog.hpp>

namespace sdpa { namespace modules {
  class IModule
  {
  public:
    virtual ~IModule() throw () {}

    virtual void name(const std::string &) = 0;

    virtual void add_function(const std::string &function
                            , GenericFunction f)
                            throw (DuplicateFunction, FunctionException) = 0;
    virtual void add_function(const std::string &function
                            , GenericFunction f
                            , const param_names_list_t &parameters)
                            throw (DuplicateFunction, FunctionException) = 0;
  };
}}

#endif
