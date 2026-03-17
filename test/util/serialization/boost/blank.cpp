#include <gspc/util/serialization/boost/blank.hpp>
#include <gspc/util/serialization/trivial.hpp>
#include <gspc/testing/require_compiletime.hpp>
#include <gspc/testing/require_serialized_to_id.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (is_trivial)
{
  GSPC_TESTING_COMPILETIME_REQUIRE_EQUAL
    (gspc::util::serialization::is_trivially_serializable<::boost::blank>{}, true);
}

BOOST_AUTO_TEST_CASE (empty)
{
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID ({}, ::boost::blank);
}
