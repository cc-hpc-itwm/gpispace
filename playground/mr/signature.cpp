// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_signature
#include <boost/test/unit_test.hpp>

#include <we/type/signature.hpp>
#include <we/type/signature/show.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (signature_show)
{
#define CHECK(_expected,_sig...)                \
  {                                             \
    using pnet::type::signature::show;          \
                                                \
    std::ostringstream oss;                     \
                                                \
    oss << show (signature_type (_sig));        \
                                                \
    BOOST_CHECK_EQUAL (oss.str(), _expected);   \
  }

  using pnet::type::signature::signature_type;
  using pnet::type::signature::signature_structured_type;
  using pnet::type::signature::structure_type;
  using pnet::type::signature::field_type;

  CHECK ("float", std::string ("float"));

  structure_type f;

  CHECK ("s :: []", signature_structured_type (std::make_pair ("s", f)));

  f.push_back (std::make_pair (std::string ("x"), std::string ("float")));

  CHECK ("s :: [x :: float]", signature_structured_type (std::make_pair ("s", f)));

  f.push_back (std::make_pair (std::string ("y"), std::string ("float")));

  CHECK ( "point2D :: [x :: float, y :: float]"
        , signature_structured_type (std::make_pair ("point2D", f))
        );

  structure_type ps;

  ps.push_back (std::make_pair (std::string ("p"), std::string ("point2D")));
  ps.push_back (signature_structured_type (std::make_pair ("q", f)));

  CHECK ( "line2D :: [p :: point2D, q :: [x :: float, y :: float]]"
        , signature_structured_type (std::make_pair ("line2D", ps))
        );
#undef CHECK
}
