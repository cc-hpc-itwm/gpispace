// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_GET_HPP
#define _WE_TYPE_VALUE_GET_HPP

#include <we/type/value.hpp>
#include <we/type/signature.hpp>
#include <we/type/literal.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace value
{
  namespace visitor
  {
    namespace exception
    {
      class missing_field : public std::runtime_error
      {
      public:
        missing_field (const std::string&);
      };

      class cannot_get_field_from_literal : public std::runtime_error
      {
      public:
        cannot_get_field_from_literal ( const std::string&
                                      , const literal::type&
                                      );
      };

      class bad_get : public std::runtime_error
      {
      public:
        bad_get (const value::structured_t& o)
          : std::runtime_error
            ( ( boost::format ("bad get: expexted literal, got %1%") % 1
              ).str()
            )
        {}
      };

      class empty_path : public std::runtime_error
      {
      public:
        empty_path();
      };
    }

    class get_field : public boost::static_visitor<const type&>
    {
    public:
      get_field (const path_type&);
      get_field (const signature::field_name_t&);

      const type& operator() (const structured_t&);
      const type& operator() (const literal::type&);

    private:
      path_type _path;
      path_type::const_iterator _pos;

      std::string name() const;
    };

    template <typename T>
    class get : public boost::static_visitor<T>
    {
    public:
      typedef T result_type;

      result_type operator() (const literal::type& literal) const
      {
        return boost::get<T> (literal);
      }

      result_type operator() (const value::structured_t& o) const
      {
        throw exception::bad_get (o);
      }
    };

    template <typename T>
    class get_ref : public boost::static_visitor<T>
    {
    public:
      typedef T result_type;

      result_type operator() (literal::type& literal) const
      {
        return boost::get<T> (literal);
      }

      result_type operator() (value::structured_t& o) const
      {
        throw exception::bad_get (o);
      }
    };
  }

  template <typename T, typename V>
  const T& get (const V& v)
  {
    return boost::apply_visitor (visitor::get<T const&>(), v);
  }

  template <typename T>
  T& get_ref (type& v)
  {
    return boost::apply_visitor (visitor::get_ref<T&>(), v);
  }

  const type& get_field (const signature::field_name_t&, const type&);
  const type& get_field (const path_type&, const type&);

  // to get something means to get a literal value...
  template<typename T>
  const T& get (const signature::field_name_t& field, const type& v)
  {
    return get<T, type> (get_field (field, v));
  }

  template<typename T>
  const T& get (const path_type& path, const type& v)
  {
    return get<T, type> (get_field (path, v));
  }

  // ...but not when stated explicitely to be a value::type
  template<>
  const type& get<type> (const signature::field_name_t&, const type&);

  template<>
  const type& get<type> (const path_type&, const type&);
}

#endif
