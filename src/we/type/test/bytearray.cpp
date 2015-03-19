#define BOOST_TEST_MODULE WeTypeBytearrayTest
#include <boost/test/unit_test.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/test/printer/tuple.hpp>
#include <fhg/util/boost/test.hpp>

#include <we/type/bytearray.hpp>

#include <util-generic/testing/random_string.hpp>

#include <inttypes.h>

BOOST_AUTO_TEST_CASE (ba_char)
{
  char buf[10];

  for (int i (0); i < 10; ++i)
    {
      buf[i] = i + 'a';
    }

  const we::type::bytearray x (buf, 10);

  std::fill (buf, buf + 10, 0);

  BOOST_CHECK_EQUAL (x.copy (buf, 10), 10);
  BOOST_CHECK_EQUAL (std::string (buf, buf + 10), "abcdefghij");
  BOOST_CHECK_EQUAL (x.copy (buf, 12), 10);
}
BOOST_AUTO_TEST_CASE (ba_uint64_t)
{
  uint64_t v (1UL << 63);

  const we::type::bytearray x (&v);

  v = 0;

  BOOST_CHECK_EQUAL (x.copy (&v), sizeof (uint64_t));
  BOOST_CHECK_EQUAL (v, (1UL << 63));
}
BOOST_AUTO_TEST_CASE (ba_convert)
{
  uint64_t v (1UL << 63);

  const we::type::bytearray x (&v);

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

BOOST_AUTO_TEST_CASE (ba_to_string_after_ctor_string_is_id)
{
  std::string const s (fhg::util::testing::random_string());

  BOOST_REQUIRE_EQUAL (we::type::bytearray (s).to_string(), s);
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

  p () = default;
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

  q () = default;
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

  we::type::bytearray y;

  {
    const we::type::bytearray x (buf, 10);

    y = x;
  }

  std::fill (buf, buf + 10, 0);

  BOOST_CHECK_EQUAL (y.copy (buf, 10), 10);
  BOOST_CHECK_EQUAL (std::string (buf, buf + 10), "abcdefghij");
  BOOST_CHECK_EQUAL (y.copy (buf, 12), 10);
}

BOOST_AUTO_TEST_CASE (ba_assign_from_alien)
{
  we::type::bytearray ba;

  ba = 1.0f;

  float f;

  BOOST_CHECK_EQUAL (ba.copy (&f), sizeof(float));
  BOOST_CHECK_EQUAL (f, 1.0f);
}

namespace
{
  template<typename T> void use_ctor_and_copy_and_require_equal (T x)
  {
    we::type::bytearray ba (x);
    T y;
    ba.copy (&y);
    BOOST_REQUIRE_EQUAL (x, y);
  }

  struct pod_t
  {
    int x;
    int y;
  };
  bool operator== (pod_t const& lhs, pod_t const& rhs)
  {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
}

FHG_BOOST_TEST_LOG_VALUE_PRINTER (pod_t, os, pod)
{
  os << pod.x << ", " << pod.y;
}

BOOST_AUTO_TEST_CASE (ctor_from_T)
{
  use_ctor_and_copy_and_require_equal (1.0f);
  use_ctor_and_copy_and_require_equal (pod_t {1, 15});
  use_ctor_and_copy_and_require_equal (std::make_tuple (false, 8.0, 'a'));
}
