#define BOOST_TEST_MODULE WeTypeBytearrayTest
#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>

#include <we/type/bytearray.hpp>

#include <inttypes.h>

BOOST_AUTO_TEST_CASE (ba_char)
{
  char buf[10];

  for (int i (0); i < 10; ++i)
    {
      buf[i] = i + 'a';
    }

  const bytearray::type x (buf, 10);

  std::fill (buf, buf + 10, 0);

  BOOST_CHECK_EQUAL (x.copy (buf, 10), 10);
  BOOST_CHECK_EQUAL (std::string (buf, buf + 10), "abcdefghij");
  BOOST_CHECK_EQUAL (x.copy (buf, 12), 10);
}
BOOST_AUTO_TEST_CASE (ba_uint64_t)
{
  uint64_t v (1UL << 63);

  const bytearray::type x (&v);

  v = 0;

  BOOST_CHECK_EQUAL (x.copy (&v), sizeof (uint64_t));
  BOOST_CHECK_EQUAL (v, (1UL << 63));
}
BOOST_AUTO_TEST_CASE (ba_convert)
{
  uint64_t v (1UL << 63);

  const bytearray::type x (&v);

  BOOST_REQUIRE_EQUAL (sizeof (uint64_t), 8);

  char buf[8];

  BOOST_CHECK_EQUAL (x.copy (buf, 8), 8);
  BOOST_CHECK_EQUAL (buf[0], 0);
  BOOST_CHECK_EQUAL (buf[1], 0);
  BOOST_CHECK_EQUAL (buf[2], 0);
  BOOST_CHECK_EQUAL (buf[3], 0);
  BOOST_CHECK_EQUAL (buf[4], 0);
  BOOST_CHECK_EQUAL (buf[5], 0);
  BOOST_CHECK_EQUAL (buf[6], 0);
  BOOST_CHECK_EQUAL (buf[7], -128);

  v = 0;

  BOOST_CHECK_EQUAL (x.copy (&v), 8);
  BOOST_CHECK_EQUAL (v, (1UL << 63));
}

#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

struct p
{
  double _x;
  long _y;

  p () : _x(), _y() {}
  p (const double & x, const long & y) : _x (x), _y (y) {}

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP (_x);
    ar & BOOST_SERIALIZATION_NVP (_y);
  }

  bool operator == (const p & other)
  {
    return _x == other._x && _y == other._y;
  }
};

struct q
{
  p _p;
  std::string _s;

  q () : _p(), _s() {}
  q (const p & p, const std::string & s) : _p(p), _s(s) {}

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP (_p);
    ar & BOOST_SERIALIZATION_NVP (_s);
  }

  bool operator == (const q & other)
  {
    return _p == other._p && _s == other._s;
  }
};

BOOST_AUTO_TEST_CASE (ba_assign_from_ba)
{
  char buf[10];

  for (int i (0); i < 10; ++i)
    {
      buf[i] = i + 'a';
    }

  bytearray::type y;

  {
    const bytearray::type x (buf, 10);

    y = x;
  }

  std::fill (buf, buf + 10, 0);

  BOOST_CHECK_EQUAL (y.copy (buf, 10), 10);
  BOOST_CHECK_EQUAL (std::string (buf, buf + 10), "abcdefghij");
  BOOST_CHECK_EQUAL (y.copy (buf, 12), 10);
}

BOOST_AUTO_TEST_CASE (ba_assign_from_alien)
{
  bytearray::type ba;

  ba = 1.0f;

  float f;

  BOOST_CHECK_EQUAL (ba.copy (&f), sizeof(float));
  BOOST_CHECK_EQUAL (f, 1.0f);
}
