#include <boost/test/unit_test.hpp>

#include <we/type/memory_buffer_info.hpp>

#include <util-generic/testing/random_integral.hpp>
#include <util-generic/testing/random_string.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

BOOST_AUTO_TEST_CASE (size_is_stored)
{
  std::string const size (fhg::util::testing::random_string());
  bool const read_only (fhg::util::testing::random_integral<unsigned long>()%2);

  BOOST_REQUIRE_EQUAL
    ( size
    , we::type::memory_buffer_info
      ( size
      , read_only
      ).size()
    );
}

BOOST_AUTO_TEST_CASE (read_only_is_stored)
{
  std::string const size (fhg::util::testing::random_string());
  bool const read_only (fhg::util::testing::random_integral<unsigned long>()%2);

  BOOST_REQUIRE_EQUAL
    ( read_only
    , we::type::memory_buffer_info
      ( size
      , read_only
      ).read_only()
    );
}

BOOST_AUTO_TEST_CASE (memory_buffer_info_serialization)
{
  std::stringstream strstream;
  boost::archive::text_oarchive oar (strstream);

  std::string const size (fhg::util::testing::random_string());
  bool const read_only
    (fhg::util::testing::random_integral<unsigned long>()%2);

  oar << we::type::memory_buffer_info (size, read_only);

  boost::archive::text_iarchive iar (strstream);

  we::type::memory_buffer_info buffer_info;
  iar >> buffer_info;

  BOOST_REQUIRE_EQUAL (buffer_info.size(), size);
  BOOST_REQUIRE_EQUAL (buffer_info.read_only(), read_only);
}
