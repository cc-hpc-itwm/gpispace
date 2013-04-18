#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/show.hpp>
#include <boost/format.hpp>
#include <iostream>

using pnet::type::value::value_type;
using pnet::type::value::structured_type;
using pnet::type::value::poke;
using pnet::type::value::peek;

namespace
{
  template<typename T>
  class visitor_field : public boost::static_visitor<const T&>
  {
  public:
    visitor_field (const std::string& field)
      : _field (field)
    {}
    const T& operator() (const structured_type& m) const
    {
      const structured_type::const_iterator pos (m.find (_field));

      if (pos == m.end())
      {
        throw std::runtime_error ("missing field");
      }

      const T* x (boost::get<const T> (&pos->second));

      if (!x)
      {
        throw std::runtime_error ("type error");
      }

      return *x;
    }
    template<typename X>
    const T& operator() (const X& x) const
    {
      throw std::runtime_error ("missing field");
    }

  private:
    const std::string _field;
  };
  template<>
  class visitor_field<value_type> : public boost::static_visitor<const value_type&>
  {
  public:
    visitor_field (const std::string& field)
      : _field (field)
    {}
    const value_type& operator() (const structured_type& m) const
    {
      const structured_type::const_iterator pos (m.find (_field));

      if (pos == m.end())
      {
        throw std::runtime_error ("missing field");
      }

      return pos->second;
    }
    template<typename X>
    const value_type& operator() (const X& x) const
    {
      throw std::runtime_error ("missing field");
    }

  private:
    const std::string _field;
  };
}

template<typename T>
const T& field (const std::string& f, const value_type& v)
{
  return boost::apply_visitor (visitor_field<T> (f), v);
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

        type (const value_type& v)
          : f (field<float> ("f", v))
          , s (field<std::string> ("s", v))
        {}
      };
    }

    struct type
    {
      x::type x;
      int i;

      type (const value_type& v)
        : x (field<value_type> ("x", v))
        , i (field<int>("i", v))
      {}
    };
  }

  struct type
  {
    std::list<value_type> l;
    y::type y;

    type (const value_type& v)
      : l (field<std::list<value_type> > ("l", v))
      , y (field<value_type> ("y", v))
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

  std::cout << v << std::endl;

  z::type z (v);

  std::cout << z.y.x.f << std::endl;
  std::cout << z.y.x.s << std::endl;
  std::cout << z.y.i << std::endl;
  std::cout << z.l.size() << std::endl;
}
