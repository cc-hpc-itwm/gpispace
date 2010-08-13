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

    typedef expr::eval::context input_t;
    typedef expr::eval::context output_t;

    typedef void (*InitializeFunction)(IModule*, unsigned int);
    typedef void (*FinalizeFunction)(IModule*);
    typedef void (*WrapperFunction)(void *, const input_t &, output_t &);

    typedef std::list<std::string> param_names_list_t;
    typedef std::pair<WrapperFunction, param_names_list_t> parameterized_function_t;

    // ********************************************************************** //
    // PUT

    // on port, complete token
    inline void put ( output_t & o
                    , const std::string & key
                    , const value::type & val
                    )
    {
      o.bind (key, val);
    }

    // on port, subtoken by path
    inline void put ( output_t & o
                    , const std::string & key
                    , const value::path_type & path
                    , const value::type & val
                    )
    {
      o.bind<value::path_type> (key, path, val);
    }

    inline void put ( output_t & o
                    , const std::string & key
                    , const std::string & path
                    , const value::type & val
                    )
    {
      o.bind<std::string> (key, path, val);
    }

    // on port, complete literal
    template <typename T>
    inline void put ( output_t & o
                    , const std::string & key
                    , const T & val
                    )
    {
      put (o, key, value::type (val));
    }

    // on port, subliteral by string-path
    template<typename T>
    inline void put ( output_t & o
                    , const std::string & key
                    , const value::path_type & path
                    , const T & val
                    )
    {
      put (o, key, path, value::type (val));
    }

    template <typename T>
    inline void put ( output_t & o
                    , const std::string & key
                    , const std::string & path
                    , const T & val
                    )
    {
      put (o, key, path, value::type (val));
    }

    // ********************************************************************** //
    // GET

    // getting something means to get a literal value...
    template <typename T>
    inline typename value::visitor::get<T const &>::result_type
    get (const input_t & i, const std::string & key)
    {
      return value::get<T>(i.value(key));
    }

    // ...but not when stated explicitely be a value::type
    template<>
    inline const value::type &
    get<value::type> (const input_t & i, const std::string & key)
    {
      return i.value (key);
    }

    // get with an additional path into the value
    template <typename T>
    inline typename value::visitor::get<T const &>::result_type
    get ( const input_t & i
        , const std::string & key
        , const value::path_type & path_in_value
        )
    {
      return value::get<T>(path_in_value, i.value (key));
    }

    template <typename T>
    inline typename value::visitor::get<T const &>::result_type
    get ( const input_t & i
        , const std::string & key
        , const std::string & path_in_value
        )
    {
      return value::get<T>(path_in_value, i.value (key));
    }

    // get from an earlier extracted value::type
    template <typename T>
    inline typename value::visitor::get<T const &>::result_type
    get (const value::type & v)
    {
      return value::get<T>(v);
    }

    template <typename T>
    inline typename value::visitor::get<T const &>::result_type
    get ( const value::type & v
        , const value::path_type & path_in_value
        )
    {
      return value::get<T>(path_in_value, v);
    }

    template <typename T>
    inline typename value::visitor::get<T const &>::result_type
    get ( const value::type & v
        , const std::string & path_in_value
        )
    {
      return value::get<T>(path_in_value, v);
    }
  }
}

#endif
