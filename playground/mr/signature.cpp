// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_signature
#include <boost/test/unit_test.hpp>

#include <we/type/signature.hpp>
#include <we/type/signature/show.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (signature_show)
{
  using pnet::type::signature::signature_type;
  using pnet::type::signature::show;

#define CHECK(_expected,_sig...)                \
  {                                             \
    std::ostringstream oss;                     \
                                                \
    oss << show (signature_type (_sig));        \
                                                \
    BOOST_CHECK_EQUAL (oss.str(), _expected);   \
  }

#undef CHECK
}
