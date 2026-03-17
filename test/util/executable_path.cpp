#include <boost/test/unit_test.hpp>

#include <gspc/util/executable_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <filesystem>

BOOST_AUTO_TEST_CASE (executable_path)
{
  BOOST_REQUIRE_EQUAL
    ( std::filesystem::canonical
      (::boost::unit_test::framework::master_test_suite().argv[0])
    , gspc::util::executable_path()
    );
}

static int symbol_in_this_binary;

BOOST_AUTO_TEST_CASE (executable_path_symbol)
{
  BOOST_REQUIRE_EQUAL
    ( std::filesystem::canonical
      (::boost::unit_test::framework::master_test_suite().argv[0])
    , gspc::util::executable_path (&symbol_in_this_binary)
    );
}
