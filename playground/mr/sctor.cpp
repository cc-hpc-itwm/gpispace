// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_value_ctor
#include <boost/test/unit_test.hpp>

#include <we/type/value/field.hpp>

#include <we/type/value/signature/of_type.hpp>
#include <we/type/value/poke.hpp>

#include <boost/serialization/nvp.hpp>
#include <we/type/value/serialize.hpp>

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
    struct type
    {
      float f;
      std::string s;

      type();
      explicit type (const pnet::type::value::value_type&);

      template<typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP (f);
        ar & BOOST_SERIALIZATION_NVP (s);
      }
    };

    pnet::type::value::value_type value (const type&);
  }

  struct type
  {
    x::type x;
    int i;

    type();
    explicit type (const pnet::type::value::value_type&);

    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP (x);
      ar & BOOST_SERIALIZATION_NVP (i);
    }
  };

  pnet::type::value::value_type value (const type&);
}

// file type/y.cpp

INCLUDE (type/y.hpp)

namespace y
{
  namespace x
  {
    type::type()
      : f()
      , s()
    {}
    type::type (const pnet::type::value::value_type& v)
      : f (pnet::type::value::field_as<float> (pnet::type::value::path ("f"), v, pnet::type::value::of_type ("float")))
      , s (pnet::type::value::field_as<std::string> (pnet::type::value::path ("s"), v, pnet::type::value::of_type ("string")))
    {}

    pnet::type::value::value_type value (const type& x)
    {
      pnet::type::value::value_type v;

      pnet::type::value::poke ("f", v, x.f);
      pnet::type::value::poke ("s", v, x.s);

      return v;
    }
  }

  type::type()
    : x()
    , i()
  {}
  type::type (const pnet::type::value::value_type& v)
    : x (pnet::type::value::field (pnet::type::value::path ("x"), v, value (x::type())))
    , i (pnet::type::value::field_as<int> (pnet::type::value::path ("i"), v, pnet::type::value::of_type ("int")))
  {}

  pnet::type::value::value_type value (const type& x)
  {
    pnet::type::value::value_type v;

    pnet::type::value::poke ("x", v, x::value (x.x));
    pnet::type::value::poke ("i", v, x.i);

    return v;
  }
}

// file: type/z.hpp
namespace z
{
  struct type
  {
    std::list<pnet::type::value::value_type> l;
    y::type y;
    y::type yy;

    type();
    explicit type (const pnet::type::value::value_type&);

    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP (l);
      ar & BOOST_SERIALIZATION_NVP (y);
      ar & BOOST_SERIALIZATION_NVP (yy);
    }
  };

  pnet::type::value::value_type value (const type&);
}


// file: type/z.cpp

INCLUDE (type/z.hpp)

namespace z
{
  type::type()
    : l()
    , y()
    , yy()
  {}
  type::type (const pnet::type::value::value_type& v)
    : l (pnet::type::value::field_as<std::list<pnet::type::value::value_type> > (pnet::type::value::path ("l"), v, pnet::type::value::of_type ("list")))
    , y (pnet::type::value::field (pnet::type::value::path ("y"), v, value (y::type())))
    , yy (pnet::type::value::field (pnet::type::value::path ("yy"), v, value (y::type())))
  {}

  pnet::type::value::value_type value (const type& x)
  {
    pnet::type::value::value_type v;

    pnet::type::value::poke ("l", v, x.l);
    pnet::type::value::poke ("y", v, y::value (x.y));
    pnet::type::value::poke ("yy", v, y::value (x.yy));

    return v;
  }
}

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

BOOST_AUTO_TEST_CASE (ctor)
{
  using pnet::type::value::value_type;
  using pnet::type::value::poke;

  value_type v;

  poke ("l", v, std::list<value_type>());
  poke ("y.x.f", v, 1.0f);
  poke ("y.x.s", v, std::string ("string"));
  poke ("y.i", v, 42);
  poke ("yy.x.f", v, 1.0f);
  poke ("yy.x.s", v, std::string ("string"));
  poke ("yy.i", v, 42);

  std::ostringstream oss;

  {
    const z::type z (v);

    BOOST_CHECK (v == value (z));

    boost::archive::text_oarchive oa (oss);
    oa << z;
  }

  {
    std::istringstream iss (oss.str());
    boost::archive::text_iarchive ia (iss);
    z::type z;

    ia >> z;

    BOOST_CHECK (v == value (z));
  }
}
