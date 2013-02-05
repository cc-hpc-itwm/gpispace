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
    template<typename Path>
    void put ( output_t& o
             , const std::string& key
             , const Path& path
             , const value::type& val
             )
    {
      value::container::bind (o, key, path, val);
    }

    // get with an additional path into the value
    template<typename T, typename Path>
    const T& get ( const input_t& i
                 , const std::string& key
                 , const Path& path_in_value
                 )
    {
      return value::get<T>(path_in_value, value::container::value (i, key));
    }

    // get from an earlier extracted value::type
    template <typename T>
    const T& get (const value::type & v)
    {
      return value::get<T>(v);
    }

    template <typename T>
    const T& get (const value::type& v, const value::path_type& path_in_value)
    {
      return value::get<T>(path_in_value, v);
    }

    template <typename T>
    const T& get (const value::type& v, const std::string& path_in_value)
    {
      return value::get<T>(path_in_value, v);
    }
  }
}

#endif
