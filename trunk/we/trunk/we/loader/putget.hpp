// mirko.rahn@itwm.fhg.de

#ifndef WE_LOADER_PUTGET_HPP
#define WE_LOADER_PUTGET_HPP 1

#include <we/loader/types.hpp>

#include <we/type/value.hpp>
#include <we/type/value/put.hpp>
#include <we/type/value/get.hpp>

#include <we/type/value/container/bind.hpp>

namespace we
{
  namespace loader
  {
    // ********************************************************************** //
    // PUT

    // on port, subtoken by path
    inline void put ( output_t & o
                    , const std::string & key
                    , const value::path_type & path
                    , const value::type & val
                    )
    {
      value::container::bind<value::path_type> (o, key, path, val);
    }

    inline void put ( output_t & o
                    , const std::string & key
                    , const std::string & path
                    , const value::type & val
                    )
    {
      value::container::bind<std::string> (o, key, path, val);
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

    // get with an additional path into the value
    template <typename T>
    inline typename value::visitor::get<T const &>::result_type
    get ( const input_t & i
        , const std::string & key
        , const value::path_type & path_in_value
        )
    {
      return value::get<T>(path_in_value, value::container::value (i, key));
    }

    template <typename T>
    inline typename value::visitor::get<T const &>::result_type
    get ( const input_t & i
        , const std::string & key
        , const std::string & path_in_value
        )
    {
      return value::get<T>(path_in_value, value::container::value (i, key));
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
