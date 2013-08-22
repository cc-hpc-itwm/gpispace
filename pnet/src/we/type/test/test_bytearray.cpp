#define BOOST_TEST_MODULE WeTypeBytearrayTest
#include <boost/test/unit_test.hpp>

#include <fhglog/minimal.hpp>

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

  BOOST_CHECK (x.copy (buf, 10) == 10);
  BOOST_CHECK (std::string (buf, buf + 10) == "abcdefghij");
  BOOST_CHECK (x.copy (buf, 12) == 10);
}
BOOST_AUTO_TEST_CASE (ba_uint64_t)
{
  uint64_t v (1UL << 63);

  const bytearray::type x (&v);

  v = 0;

  BOOST_CHECK (x.copy (&v) == sizeof (uint64_t));
  BOOST_CHECK (v == (1UL << 63));
}
BOOST_AUTO_TEST_CASE (ba_convert)
{
  uint64_t v (1UL << 63);

  const bytearray::type x (&v);

  BOOST_REQUIRE (sizeof (uint64_t) == 8);

  char buf[8];

  BOOST_CHECK (x.copy (buf, 8) == 8);
  BOOST_CHECK (buf[0] == 0);
  BOOST_CHECK (buf[1] == 0);
  BOOST_CHECK (buf[2] == 0);
  BOOST_CHECK (buf[3] == 0);
  BOOST_CHECK (buf[4] == 0);
  BOOST_CHECK (buf[5] == 0);
  BOOST_CHECK (buf[6] == 0);
  BOOST_CHECK (buf[7] == -128);

  v = 0;

  BOOST_CHECK (x.copy (&v) == 8);
  BOOST_CHECK (v == (1UL << 63));
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

BOOST_AUTO_TEST_CASE (ba_list_of_something)
{
  typedef std::vector<q> vec_t;

  vec_t v;

  q q1 (p (3.1, 1), "one");
  q q2 (p (2.7, 2), "two");
  q q3 (p (9.0, 3), "three");

  v.push_back (q1);
  v.push_back (q2);
  v.push_back (q3);

  std::ostringstream oss;

  const bytearray::encoder<vec_t> encoder (v);
  const bytearray::type ba_encoded (encoder.bytearray());

  const bytearray::decoder<vec_t> decoder (ba_encoded);
  const vec_t & w (decoder.value());

  BOOST_CHECK (v.size() == w.size());

  for (std::size_t i (0); i < v.size(); ++i)
  {
    BOOST_CHECK (v[i] == w[i]);
  }
}

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

  BOOST_CHECK (y.copy (buf, 10) == 10);
  BOOST_CHECK (std::string (buf, buf + 10) == "abcdefghij");
  BOOST_CHECK (y.copy (buf, 12) == 10);
}

BOOST_AUTO_TEST_CASE (ba_assign_from_alien)
{
  bytearray::type ba;

  ba = 1.0f;

  float f;

  BOOST_CHECK (ba.copy (&f) == sizeof(float));
  BOOST_CHECK (f == 1.0f);
}
