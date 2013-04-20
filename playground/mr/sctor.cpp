// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value.hpp>
#include <we/type/value/exception.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/signature.hpp>
#include <we/type/value/signature/of_type.hpp>

#include <iostream>

using pnet::type::value::value_type;
using pnet::type::value::signature_type;
using pnet::type::value::structured_type;
using pnet::type::value::peek;
using pnet::type::value::poke;
using pnet::type::value::of_type;
using pnet::type::value::exception::missing_field;
using pnet::type::value::exception::type_mismatch;

static std::list<std::string> init_ctor_path()
{
  return std::list<std::string>();
}
static std::list<std::string>& ctor_path()
{
  static std::list<std::string> p (init_ctor_path());

  return p;
}
class append
{
public:
  append (std::list<std::string>& path, const std::string& x)
    : _path (path)
  {
    _path.push_back (x);
  }
  ~append()
  {
    _path.pop_back();
  }
  operator std::list<std::string>&() const
  {
    return _path;
  }
private:
  std::list<std::string>& _path;
};

template<typename T>
const T& field ( const std::list<std::string>& path
               , const std::string& f
               , const value_type& v
               , const signature_type& signature
               )
{
  const value_type& field (field<value_type> (path, f, v, signature));

  const T* x (boost::get<const T> (&field));

  if (!x)
  {
    throw type_mismatch (signature, field, path);
  }

  return *x;
}

template<>
const value_type& field<value_type> ( const std::list<std::string>& path
                                    , const std::string& f
                                    , const value_type& v
                                    , const signature_type& signature
                                    )
{
  boost::optional<const value_type&> field (peek (f, v));

  if (!field)
  {
    throw missing_field (signature, path);
  }

  return *field;
}

// to be generated
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
          : f (field<float> (append (ctor_path(), "f"), "f", v, of_type ("float")))
          , s (field<std::string> (append (ctor_path(), "s"), "s", v, of_type ("string")))
        {}
        static signature_type signature()
        {
          static signature_type sig (type::init_signature());

          return sig;
        }

      private:
        static signature_type init_signature()
        {
          signature_type s;

          poke ("f", s, of_type ("float"));
          poke ("s", s, of_type ("string"));

          return s;
        }
      };
    }

    struct type
    {
      x::type x;
      int i;

      type (const value_type& v)
        : x (field<value_type> (append (ctor_path(), "x"), "x", v, x::type::signature()))
        , i (field<int> (append (ctor_path(), "i"), "i", v, of_type ("int")))
      {}
      static signature_type& signature()
      {
        static signature_type sig (type::init_signature());

        return sig;
      }

    private:
      static signature_type init_signature()
      {
        signature_type s;

        poke ("x", s, x::type::signature());
        poke ("i", s, of_type ("int"));

        return s;
      }
    };
  }

  struct type
  {
    std::list<value_type> l;
    y::type y;
    y::type yy;

    type (const value_type& v)
      : l (field<std::list<value_type> > (append (ctor_path(), "l"), "l", v, of_type ("list")))
      , y (field<value_type> (append (ctor_path(), "y"), "y", v, y::type::signature()))
      , yy (field<value_type> (append (ctor_path(), "yy"), "yy", v, y::type::signature()))
    {}
    static const signature_type& signature()
    {
      static signature_type s (type::init_signature());

      return s;
    }

  private:
    static signature_type init_signature()
    {
      signature_type s;

      poke ("l", s, of_type ("list"));
      poke ("y", s, y::type::signature());
      poke ("yy", s, y::type::signature());

      return s;
    }
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
