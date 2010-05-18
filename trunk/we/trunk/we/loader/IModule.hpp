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

#ifndef WE_LOADER_IMODULE_HPP
#define WE_LOADER_IMODULE_HPP 1

#include <string>

#include <we/loader/types.hpp>
#include <we/loader/exceptions.hpp>

namespace we {
  namespace loader {
    class IModule
    {
    public:
      virtual ~IModule() throw () {}

      virtual void name (const std::string &name) = 0;

      virtual void add_function( const std::string &function
                               , WrapperFunction f
                               ) throw ( DuplicateFunction
                                       , FunctionException
                                       ) = 0;

      virtual void add_function( const std::string &function
                               , WrapperFunction f
                               , const param_names_list_t &parameters
                               ) throw ( DuplicateFunction
                                       , FunctionException
                                       ) = 0;

      virtual void* state (void*) = 0;
      virtual void* state (void) = 0;
    };
  }
}

#endif
