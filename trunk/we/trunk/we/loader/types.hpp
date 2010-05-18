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

#include <we/we.hpp>
#include <vector>

namespace we
{
  namespace loader
  {
    class IModule;

    typedef expr::eval::context <signature::field_name_t> input_t;
    typedef std::vector <std::pair<value::type, std::string> > output_t;

    typedef void (*InitializeFunction)(IModule*, unsigned int);
    typedef void (*FinalizeFunction)(IModule*);
    typedef void (*WrapperFunction)(void *, const input_t &, output_t &);

    typedef std::list<std::string> param_names_list_t;
    typedef std::pair<WrapperFunction, param_names_list_t> parameterized_function_t;

    inline void put_output (output_t & o, const std::string & key, const value::type & val)
    {
      o.push_back (output_t::value_type (val, key));
    }

    template <typename T>
    inline
    void put_output (output_t & o, const std::string & key, const T & val)
    {
      put_output(o, key, value::type (val));
    }

    template <typename T>
    inline
    typename value::visitor::get_literal_value<T>::result_type const & get_input (const input_t & i, const std::string & key)
    {
      return value::get_literal_value<T>(i.value(key));
    }
  }
}

#endif
