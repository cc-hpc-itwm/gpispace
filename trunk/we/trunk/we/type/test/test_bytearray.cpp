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

template<typename Iarch, typename Oarch>
void check_char ()
{
  std::ostringstream oss;

  {
    char buf[10];

    for (int i (0); i < 10; ++i)
      {
        buf[i] = i;
      }

    const bytearray::type x (buf, 10);

    Oarch oa (oss, boost::archive::no_header);

    oa << BOOST_SERIALIZATION_NVP(x);
  }

  {
    std::istringstream iss (oss.str());

    Iarch ia (iss, boost::archive::no_header);

    bytearray::type x;

    ia >> BOOST_SERIALIZATION_NVP(x);

    char buf[10];

    std::fill (buf, buf + 10, -1);

    BOOST_CHECK (x.copy (buf, 10) == 10);

    for (int i (0); i < 10; ++i)
      {
        BOOST_CHECK (buf[i] == i);
      }
  }
}
BOOST_AUTO_TEST_CASE (ba_serialize_char_text)
{
  check_char<boost::archive::text_iarchive, boost::archive::text_oarchive> ();
}
BOOST_AUTO_TEST_CASE (ba_serialize_char_xml)
{
  check_char<boost::archive::xml_iarchive, boost::archive::xml_oarchive> ();
}
BOOST_AUTO_TEST_CASE (ba_serialize_char_binary)
{
  check_char< boost::archive::binary_iarchive
            , boost::archive::binary_oarchive
            > ();
}

template<typename Iarch, typename Oarch>
void check_uint64_t ()
{
  std::ostringstream oss;

  {
    uint64_t v (1UL << 63);

    const bytearray::type x (&v);

    Oarch oa (oss, boost::archive::no_header);

    oa << BOOST_SERIALIZATION_NVP(x);
  }

  {
    std::istringstream iss (oss.str());

    Iarch ia (iss, boost::archive::no_header);

    bytearray::type x;

    ia >> BOOST_SERIALIZATION_NVP(x);

    uint64_t v (0);

    BOOST_CHECK (x.copy (&v) == sizeof (uint64_t));
    BOOST_CHECK (v == (1UL << 63));
  }
}
BOOST_AUTO_TEST_CASE (ba_serialize_uint64_t_text)
{
  check_uint64_t<boost::archive::text_iarchive, boost::archive::text_oarchive> ();
}
BOOST_AUTO_TEST_CASE (ba_serialize_uint64_t_xml)
{
  check_uint64_t<boost::archive::xml_iarchive, boost::archive::xml_oarchive> ();
}
BOOST_AUTO_TEST_CASE (ba_serialize_uint64_t_binary)
{
  check_uint64_t< boost::archive::binary_iarchive
                , boost::archive::binary_oarchive
                > ();
}
