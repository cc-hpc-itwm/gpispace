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

#ifndef WE_LOADER_TYPES_HPP
#define WE_LOADER_TYPES_HPP 1

#include <we/type/value.hpp>
#include <we/type/value/cpp/get.hpp>

#include <we/expr/eval/context.hpp>

#include <vector>
#include <list>

#include <we2/type/compat.hpp>

namespace we
{
  namespace loader
  {
    class IModule;

    typedef expr::eval::context input_t;
    typedef expr::eval::context output_t;

    typedef void (*InitializeFunction)(IModule*, unsigned int);
    typedef void (*FinalizeFunction)(IModule*);
    typedef void (*WrapperFunction)(void *, const input_t &, output_t &);

    typedef std::list<std::string> param_names_list_t;
    typedef std::pair<WrapperFunction, param_names_list_t> parameterized_function_t;

    // getting something means to get a literal value...
    template <typename T>
      inline T
      get (const input_t & i, const std::string & key)
    {
      return boost::get<T> (i.value (key));
    }

    // ...but not when stated explicitely be a value::type
    template<>
      inline value::type
      get<value::type> (const input_t & i, const std::string & key)
    {
      return pnet::type::compat::COMPAT (i.value (key));
    }
  }
}

#endif
