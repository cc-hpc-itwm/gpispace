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

#include <we/type/value/container/container.hpp>

#include <we/type/literal.hpp>

#include <vector>
#include <list>

namespace we
{
  namespace loader
  {
    class IModule;

    typedef value::container::type input_t;
    typedef value::container::type output_t;

    typedef void (*InitializeFunction)(IModule*, unsigned int);
    typedef void (*FinalizeFunction)(IModule*);
    typedef void (*WrapperFunction)(void *, const input_t &, output_t &);

    typedef std::list<std::string> param_names_list_t;
    typedef std::pair<WrapperFunction, param_names_list_t> parameterized_function_t;

    inline void put ( output_t & o
                    , const std::string & key
                    , const value::type & val
                    )
    {
      value::container::bind (o, key, val);
    }

    // getting something means to get a literal value...
    template <typename T>
    inline typename value::cpp::get<T const &>::result_type
    get (const input_t & i, const std::string & key)
    {
      return value::get<T>(value::container::value (i, key));
    }

    // ...but not when stated explicitely be a value::type
    template<>
    inline const value::type &
    get<value::type> (const input_t & i, const std::string & key)
    {
      return value::container::value (i, key);
    }
  }
}

#endif
