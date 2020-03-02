// mirko.rahn@itwm.fraunhofer.de

#include <boost/test/unit_test.hpp>

#include <xml/parse/type/memory_buffer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random/string.hpp>
#include <fhg/util/xml.hpp>

#include <boost/format.hpp>

BOOST_AUTO_TEST_CASE (name_is_stored)
{
  std::string const name (fhg::util::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( name
    , xml::parse::type::memory_buffer_type
      ( xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , name
      , fhg::util::testing::random_string()
      , fhg::util::testing::random_string()
      , boost::none
      , we::type::property::type()
      ).name()
    );
}

BOOST_AUTO_TEST_CASE (size_is_stored)
{
  std::string const size (fhg::util::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( size
    , xml::parse::type::memory_buffer_type
      ( xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , fhg::util::testing::random_string()
      , size
      , fhg::util::testing::random_string()
      , boost::none
      , we::type::property::type()
      ).size()
    );
}

namespace
{
  void check_read_only_is_stored (boost::optional<bool> read_only)
  {
      BOOST_REQUIRE_EQUAL
      ( read_only
      , xml::parse::type::memory_buffer_type
        ( xml::parse::util::position_type
          (nullptr, nullptr, fhg::util::testing::random_string())
        , fhg::util::testing::random_string()
        , fhg::util::testing::random_string()
        , fhg::util::testing::random_string()
        , read_only
        , we::type::property::type()
        ).read_only()
      );
  }
}

BOOST_AUTO_TEST_CASE (read_only_is_stored)
{
  check_read_only_is_stored (boost::none);
  check_read_only_is_stored (true);
  check_read_only_is_stored (false);
}

BOOST_AUTO_TEST_CASE (name_is_unique_key)
{
  std::string const name (fhg::util::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( name
    , xml::parse::type::memory_buffer_type
      ( xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , name
      , fhg::util::testing::random_string()
      , fhg::util::testing::random_string()
      , boost::none
      , we::type::property::type()
      ).unique_key()
    );
}

namespace
{
  void check_dump (boost::optional<bool> read_only)
  {
    std::string const name (fhg::util::testing::random_identifier());
    std::string const size (fhg::util::testing::random_string_without_zero());
    std::string const alignment
      (fhg::util::testing::random_string_without_zero());

      xml::parse::type::memory_buffer_type mb
      ( xml::parse::util::position_type
      (nullptr, nullptr, fhg::util::testing::random_string())
      , name
      , size
      , alignment
      , read_only
      , we::type::property::type()
      );

    std::ostringstream oss;

    fhg::util::xml::xmlstream s (oss);

    xml::parse::type::dump::dump (s, mb);

    const std::string expected
      ( ( boost::format (R"EOS(<memory-buffer name="%1%"%3%>
  <size>%2%</size>
  <alignment>%4%</alignment>
</memory-buffer>)EOS")
        % name
        % size
        % ( !boost::get_pointer (read_only) ? std::string()
          : ( boost::format (R"EOS( read-only="%1%")EOS")
            % (*read_only ? "true" : "false")
            ).str()
          )
        % alignment
        ).str()
      );

    BOOST_REQUIRE_EQUAL (expected, oss.str());
  }
}

BOOST_AUTO_TEST_CASE (dump)
{
  check_dump (boost::none);
  check_dump (true);
  check_dump (false);
}
