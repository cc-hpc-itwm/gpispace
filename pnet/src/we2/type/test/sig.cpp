// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_signature
#include <boost/test/unit_test.hpp>

#include <sig.hpp>

#include <we2/exception.hpp>
#include <we2/type/value.hpp>
#include <we2/type/value/poke.hpp>

BOOST_AUTO_TEST_CASE (sig_value)
{
  using pnet::type::value::value_type;
  using pnet::type::value::poke;

  value_type vq;
  poke ("x", vq, 1.0f);

  BOOST_CHECK_THROW (line2D::q::type q (vq), pnet::exception::missing_field);

  poke ("y", vq, 2.0);

  BOOST_CHECK_THROW (line2D::q::type q (vq), pnet::exception::type_mismatch);

  poke ("y", vq, 2.0f);

  value_type vp;
  poke ("x", vp, 3.0f);
  poke ("y", vp, 4.0f);
  value_type vl;
  poke ("p", vl, vp);
  poke ("q", vl, vq);

  line2D::q::type q (vq);
  point2D::type p (vp);
  line2D::type l (vl);

  BOOST_CHECK (vq == line2D::q::value (q));
  BOOST_CHECK (vp == point2D::value (p));
  BOOST_CHECK (vl == line2D::value (l));
}
