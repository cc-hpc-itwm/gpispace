// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_GET_HPP
#define _WE_TYPE_VALUE_GET_HPP

#include <we/type/value.hpp>

#include <we/type/signature.hpp>
#include <we/type/literal.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/split.hpp>

#include <sstream>
#include <stdexcept>

namespace value
{
  namespace visitor
  {
    namespace exception
    {
      namespace get
      {
        class generic : public std::runtime_error
        {
        public:

        };
      }

      class missing_field : public std::runtime_error
      {
      public:
        missing_field (const signature::field_name_t & name)
          : std::runtime_error ("missing field " + name)
        {}
      };

      class cannot_get_field_from_literal : public std::runtime_error
      {
      private:
        std::string nice ( const signature::field_name_t & name
                         , const literal::type & l
                         ) const
        {
          std::ostringstream s;

          s << "cannot get field " << name << " from the literal " << l;

          return s.str();
        }

      public:
        cannot_get_field_from_literal ( const signature::field_name_t & name
                                      , const literal::type & l
                                      )
          : std::runtime_error (nice (name, l))
        {}
      };

      class bad_get : public std::runtime_error
      {
      private:
        std::string nice (const value::structured_t & o) const
        {
          std::ostringstream s;

          s << "bad get: expected literal, got " << o;

          return s.str();
        }

      public:
        bad_get (const value::structured_t & o)
          : std::runtime_error (nice (o))
        {}
      };

      class empty_path : public std::runtime_error
      {
      public:
        empty_path (void) : std::runtime_error ("get: empty path") {}
      };
    }

    // ********************************************************************* //

    class get_field : public boost::static_visitor<const type &>
    {
    private:
      typedef path_type::const_iterator it_type;

      path_type path;

      it_type pos;

      std::string name (void) const
      {
        return fhg::util::join (path.begin(), path.end(), ".");
      }

    public:
      get_field (const path_type & _path) : path (_path), pos (path.begin()) {}

      get_field (const signature::field_name_t & _name)
        : path (fhg::util::split< signature::field_name_t
                                , path_type
                                > (_name, '.')
               )
        , pos (path.begin())
      {}

      const type & operator () (const structured_t & s)
      {
        if (pos == path.end())
          {
            throw exception::empty_path ();
          }

        structured_t::const_iterator field (s.find (*pos));

        if (field == s.end())
          {
            throw exception::missing_field (name());
          }

        ++pos;

        if (pos == path.end())
          {
            return field->second;
          }
        else
          {
            return boost::apply_visitor (*this, field->second);
          }
      }

      const type & operator () (const literal::type & l)
      {
        throw exception::cannot_get_field_from_literal (name(), l);
      }
    };

    // ********************************************************************* //

    template <typename T>
    class get_literal_value : public boost::static_visitor<T>
    {
    public:
      typedef T result_type;

      result_type operator () (const literal::type & literal) const
      {
        return boost::get<T> (literal);
      }

      result_type operator () (const value::structured_t & o) const
      {
        throw exception::bad_get (o);
      }
    };

    // ********************************************************************* //
  }

  template <typename T, typename V>
  typename visitor::get_literal_value<T const &>::result_type
  get_literal_value (const V & v)
  {
    return boost::apply_visitor (visitor::get_literal_value<T const &>(), v);
  }

  inline const type &
  get_field (const signature::field_name_t & field, const type & v)
  {
    visitor::get_field get (field);

    return boost::apply_visitor (get, v);
  }

  template<typename T>
  typename visitor::get_literal_value<T const &>::result_type
  get (const signature::field_name_t & field, const type & v)
  {
    visitor::get_field get (field);

    return boost::apply_visitor ( visitor::get_literal_value<T const &>()
                                , boost::apply_visitor (get, v)
                                );
  }
}

#endif
