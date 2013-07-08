// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_compat
#include <boost/test/unit_test.hpp>

#include <we2/type/compat.hpp>

#define CHECK(x,x2)                                                     \
  {                                                                     \
    ::value::type v = x;                                                \
                                                                        \
    BOOST_CHECK (  pnet::type::value::value_type (x2)                   \
                == pnet::type::compat::COMPAT (v)                       \
                );                                                      \
  }
#define CHECK1(x) CHECK(x,x)

BOOST_AUTO_TEST_CASE (we2we2)
{
  CHECK1 (we::type::literal::control());
  CHECK1 (true);
  CHECK1 (false);
  CHECK1 (0L);
  CHECK1 (1L);
  CHECK1 (-1L);
  CHECK1 (0.0);
  CHECK1 (1.0);
  CHECK1 (-1.0);
  CHECK1 ('a');
  CHECK1 (std::string());
  CHECK1 (std::string ("beep"));
  CHECK1 (bitsetofint::type());
  CHECK1 (bitsetofint::type().ins (23));
  {
    std::list<pnet::type::value::value_type> l;
    ::literal::stack_type s;
    CHECK (s, l);
    l.push_back (23L);
    s.push_back (23L);
    //    CHECK (s, l);
  }
}
