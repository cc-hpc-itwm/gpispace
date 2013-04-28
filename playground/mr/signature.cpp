// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_signature
#include <boost/test/unit_test.hpp>

#include <we/type/signature.hpp>
#include <we/type/signature/show.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (signature_show)
{
  using pnet::type::signature::signature_type;
  using pnet::type::signature::fields_type;
  using pnet::type::signature::show;

#define CHECK(_expected,_sig...)                \
  {                                             \
    std::ostringstream oss;                     \
                                                \
    oss << show (signature_type (_sig));        \
                                                \
    BOOST_CHECK_EQUAL (oss.str(), _expected);   \
  }

  fields_type point2D;

  point2D.push_back (std::make_pair ("x", std::string ("float")));
  point2D.push_back (std::make_pair ("y", std::string ("float")));

  CHECK ("[x :: float, y :: float]", point2D);

  {
    fields_type line2D;

    line2D.push_back (std::make_pair ("p", signature_type (point2D)));
    line2D.push_back (std::make_pair ("q", signature_type (point2D)));

    CHECK ( "[p :: [x :: float, y :: float], q :: [x :: float, y :: float]]"
          , line2D
          );
  }

  {
    fields_type line2D;

    line2D.push_back (std::make_pair ("p", std::string ("point2D")));
    line2D.push_back (std::make_pair ("q", std::string ("point2D")));

    CHECK ( "[p :: point2D, q :: point2D]", line2D);
  }

#undef CHECK
}
