#include <gspc/testing/printer/type_info.hpp>

#include <gspc/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

#include <typeinfo>

BOOST_AUTO_TEST_CASE (std_type_info_name_is_printed)
{
  int i;
  std::type_info const& integer_type (typeid (int));
  std::type_info const& integer_var (typeid (i));
  std::type_info const& integer_ptr (typeid (&i));
  GSPC_TESTING_REQUIRE_PRINTED_AS (integer_type.name(), integer_type);
  GSPC_TESTING_REQUIRE_PRINTED_AS (integer_var.name(), integer_var);
  GSPC_TESTING_REQUIRE_PRINTED_AS (integer_ptr.name(), integer_ptr);
}
