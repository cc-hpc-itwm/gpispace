// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_value_ctor
#include <boost/test/unit_test.hpp>

#include <we/type/value.hpp>
#include <we/type/value/exception.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/signature.hpp>
#include <we/type/value/signature/of_type.hpp>
#include <we/type/value/signature/show.hpp>

#include <iostream>

// file: general/ctor.hpp/ctor.cpp
namespace pnet
{
  namespace type
  {
    namespace value
    {
      class path
      {
      public:
        path (const std::string& x)
          : _key (_path().insert (_path().end(), x))
          , _end (_path().end())
        {}
        ~path()
        {
          _path().pop_back();
        }
        const std::list<std::string>::const_iterator& key() const
        {
          return _key;
        }
        const std::list<std::string>::const_iterator& end() const
        {
          return _end;
        }
        const std::list<std::string>& operator() () const
        {
          return _path();
        }

      private:
        const std::list<std::string>::const_iterator _key;
        const std::list<std::string>::const_iterator _end;

        static std::list<std::string> _init_path()
        {
          return std::list<std::string>();
        }
        static std::list<std::string>& _path()
        {
          static std::list<std::string> p (_init_path());

          return p;
        }
      };

      const value_type& field ( const path& p
                              , const value_type& v
                              , const signature_type& signature
                              )
      {
        boost::optional<const value_type&> field (peek (p.key(), p.end(), v));

        if (!field)
        {
          throw exception::missing_field (signature, p());
        }

        return *field;
      }

      template<typename T>
      const T& field_as ( const path& p
                        , const value_type& v
                        , const signature_type& signature
                        )
      {
        const value_type& value (field (p, v, signature));

        const T* x (boost::get<const T> (&value));

        if (!x)
        {
          throw exception::type_mismatch (signature, value, p());
        }

        return *x;
      }
    }
  }
}

using pnet::type::value::value_type;
using pnet::type::value::signature_type;
using pnet::type::value::poke;
using pnet::type::value::of_type;
using pnet::type::value::as_signature;

using pnet::type::value::field;
using pnet::type::value::field_as;
using pnet::type::value::path;

// to be generated from
// <struct name="y">
//   <struct name="x">
//     <field name="f" type="float"/>
//     <field name="s" type="string"/>
//   </struct>
//   <field name="i" type="int"/>
// </struct>
// <struct name="z">
//   <field name="l" type="list"/>
//   <field name="y" type="y"/>
//   <field name="yy" type="y"/>
// </struct>

#define INCLUDE(...)

// file type/y.hpp
namespace y
{
  namespace x
  {
    static signature_type signature();

    struct type
    {
      float f;
      std::string s;

      type();
      explicit type (const value_type&);
    };

    value_type value (const type&);
  }

  static signature_type& signature();

  struct type
  {
    x::type x;
    int i;

    type();
    explicit type (const value_type&);
  };

  value_type value (const type&);
}

// file type/y.cpp

INCLUDE (type/y.hpp)

namespace y
{
  namespace x
  {
    namespace
    {
      static signature_type init_signature()
      {
        signature_type s;

        poke ("f", s, of_type ("float"));
        poke ("s", s, of_type ("string"));

        return s;
      }
    }

    static signature_type signature()
    {
      static signature_type sig (init_signature());

      return sig;
    }

    type::type()
      : f()
      , s()
    {}
    type::type (const value_type& v)
      : f (field_as<float> (path ("f"), v, of_type ("float")))
      , s (field_as<std::string> (path ("s"), v, of_type ("string")))
    {}

    value_type value (const type& x)
    {
      value_type v;

      poke ("f", v, x.f);
      poke ("s", v, x.s);

      return v;
    }
  }

  namespace
  {
    static signature_type init_signature()
    {
      signature_type s;

      poke ("x", s, x::signature());
      poke ("i", s, of_type ("int"));

      return s;
    }
  }

  static signature_type& signature()
  {
    static signature_type sig (init_signature());

    return sig;
  }

  type::type()
    : x()
    , i()
  {}
  type::type (const value_type& v)
    : x (field (path ("x"), v, x::signature()))
    , i (field_as<int> (path ("i"), v, of_type ("int")))
  {}

  value_type value (const type& x)
  {
    value_type v;

    poke ("x", v, x::value (x.x));
    poke ("i", v, x.i);

    return v;
  }
}

// file: type/z.hpp
namespace z
{
  static const signature_type& signature();

  struct type
  {
    std::list<value_type> l;
    y::type y;
    y::type yy;

    type();
    explicit type (const value_type&);
  };

  value_type value (const type&);
}


// file: type/z.cpp

INCLUDE (type/z.hpp)

namespace z
{
  namespace
  {
    static signature_type init_signature()
    {
      signature_type s;

      poke ("l", s, of_type ("list"));
      poke ("y", s, y::signature());
      poke ("yy", s, y::signature());

      return s;
    }
  }

  static const signature_type& signature()
  {
    static signature_type s (init_signature());

    return s;
  }

  type::type()
    : l()
    , y()
    , yy()
  {}
  type::type (const value_type& v)
    : l (field_as<std::list<value_type> > (path ("l"), v, of_type ("list")))
    , y (field (path ("y"), v, y::signature()))
    , yy (field (path ("yy"), v, y::signature()))
  {}

  value_type value (const type& x)
  {
    value_type v;

    poke ("l", v, x.l);
    poke ("y", v, y::value (x.y));
    poke ("yy", v, y::value (x.yy));

    return v;
  }
}

BOOST_AUTO_TEST_CASE (ctor)
{
  value_type v;

  poke ("l", v, std::list<value_type>());
  poke ("y.x.f", v, 1.0f);
  poke ("y.x.s", v, std::string ("string"));
  poke ("y.i", v, 42);
  poke ("yy.x.f", v, 1.0f);
  poke ("yy.x.s", v, std::string ("string"));
  poke ("yy.i", v, 42);

  z::type z (v);

  BOOST_CHECK (v == z::value (z));

  {
    std::ostringstream sig_z;

    sig_z << as_signature (z::signature());

    std::ostringstream sig_v;

    sig_v << as_signature (v);

    BOOST_CHECK_EQUAL (sig_z.str(), sig_v.str());

  }

}
