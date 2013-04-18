#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/exception.hpp>
#include <boost/format.hpp>
#include <iostream>

using pnet::type::value::value_type;
using pnet::type::value::structured_type;
using pnet::type::value::poke;
using pnet::type::value::peek;
using pnet::type::value::exception::missing_field;
using pnet::type::value::exception::type_mismatch;

namespace
{
  template<typename T>
  class visitor_field : public boost::static_visitor<const T&>
  {
  public:
    visitor_field ( const std::list<std::string>& path
                  , const std::string& field
                  )
      : _path (path)
      , _field (field)
    {
      _path.push_back (_field);
    }
    const T& operator() (const structured_type& m) const
    {
      const structured_type::const_iterator pos (m.find (_field));

      if (pos == m.end())
      {
        throw missing_field (T(), _path);
      }

      const T* x (boost::get<const T> (&pos->second));

      if (!x)
      {
        throw type_mismatch (T(), pos->second, _path);
      }

      return *x;
    }
    template<typename X>
    const T& operator() (const X& x) const
    {
      throw missing_field (T(), _path);
    }

  private:
    std::list<std::string> _path;
    const std::string _field;
  };
  template<>
  class visitor_field<value_type> : public boost::static_visitor<const value_type&>
  {
  public:
    visitor_field ( const std::list<std::string>& path
                  , const std::string& field
                  )
      : _path (path)
      , _field (field)
    {
      _path.push_back (_field);
    }
    const value_type& operator() (const structured_type& m) const
    {
      const structured_type::const_iterator pos (m.find (_field));

      if (pos == m.end())
      {
        throw missing_field (structured_type(), _path);
      }

      return pos->second;
    }
    template<typename X>
    const value_type& operator() (const X& x) const
    {
      throw missing_field (structured_type(), _path);
    }

  private:
    std::list<std::string> _path;
    const std::string _field;
  };
}

template<typename T>
const T& field ( const std::list<std::string>& path
               , const std::string& f
               , const value_type& v
               )
{
  return boost::apply_visitor (visitor_field<T> (path, f), v);
}

std::list<std::string> append ( std::list<std::string> path
                              , const std::string& x
                              )
{
  path.push_back (x);
  return path;
}

namespace z
{
  namespace y
  {
    namespace x
    {
      struct type
      {
        float f;
        std::string s;

        type ( const value_type& v
             , const std::list<std::string>& path = std::list<std::string>()
             )
          : f (field<float> (path, "f", v))
          , s (field<std::string> (path, "s", v))
        {}
      };
    }

    struct type
    {
      x::type x;
      int i;

      type ( const value_type& v
           , const std::list<std::string>& path = std::list<std::string>()
           )
        : x (field<value_type> (path, "x", v), append (path, "x"))
        , i (field<int> (path, "i", v))
      {}
    };
  }

  struct type
  {
    std::list<value_type> l;
    y::type y;
    y::type yy;

    type ( const value_type& v
         , const std::list<std::string>& path = std::list<std::string>()
         )
      : l (field<std::list<value_type> > (path, "l", v))
      , y (field<value_type> (path, "y", v), append (path, "y"))
      , yy (field<value_type> (path, "yy", v), append (path, "yy"))
    {}
  };
}

int main ()
{
  value_type v;

  poke ("l", v, std::list<value_type>());
  poke ("y.x.f", v, 1.0f);
  poke ("y.x.s", v, std::string ("string"));
  poke ("y.i", v, 42);
  poke ("yy.x.f", v, 1.0f);
  poke ("yy.x.s", v, std::string ("string"));
  poke ("yy.i", v, 42);

  std::cout << v << std::endl;

  z::type z (v);

  std::cout << z.y.x.f << std::endl;
  std::cout << z.y.x.s << std::endl;
  std::cout << z.y.i << std::endl;
  std::cout << z.l.size() << std::endl;
}
