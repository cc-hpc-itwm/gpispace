// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE context
#include <boost/test/unit_test.hpp>

#include <we/expr/eval/context.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/eq.hpp>

BOOST_AUTO_TEST_CASE (basic)
{
  typedef expr::eval::context context_t;
  typedef context_t::key_vec_t key_vec_t;

  context_t c;

  value::type x (value::read ("0L"));
  value::type y (value::read ("[]"));
  value::type z (value::read ("[x:=0.0,y:=\"a_string\"]"));
  value::type z_x (value::read ("0.0"));
  value::type z_y (value::read ("\"a_string\""));

  key_vec_t key_z_x;
  key_z_x.push_back ("z");
  key_z_x.push_back ("x");

  key_vec_t key_z_y;
  key_z_y.push_back ("z");
  key_z_y.push_back ("y");

  key_vec_t key_z_a;
  key_z_a.push_back ("z");
  key_z_a.push_back ("a");

  key_vec_t key_a_b;
  key_a_b.push_back ("a");
  key_a_b.push_back ("b");

  c.bind ("x", x);
  c.bind ("y", y);
  c.bind ("z", z);

  BOOST_REQUIRE (value::eq (c.value ("x"), x));
  BOOST_REQUIRE (value::eq (c.value ("y"), y));
  BOOST_REQUIRE (value::eq (c.value ("z"), z));
  BOOST_REQUIRE (value::eq (c.value (key_z_x), z_x));
  BOOST_REQUIRE (value::eq (c.value (key_z_y), z_y));

  try
  {
    c.value (key_z_a);
    BOOST_FAIL ("should throw");
  }
  catch (const std::runtime_error&)
  {
    /* expected */
  }
  try
  {
    c.value (key_a_b);
    BOOST_FAIL ("should throw");
  }
  catch (const std::runtime_error&)
  {
    /* expected */
  }

  {
    const double z_x_new (1.0);

    c.bind (key_z_x, z_x_new);

    BOOST_REQUIRE (value::eq (c.value (key_z_x), z_x_new));
  }

  {
    const std::string z_new ("z");

    c.bind ("z", z_new);

    BOOST_REQUIRE (value::eq (c.value ("z"), z_new));
  }

  BOOST_REQUIRE (value::eq (c.value ("x"), x));
  BOOST_REQUIRE (value::eq (c.value ("y"), y));

  try
  {
    c.value (key_z_a);
    BOOST_FAIL ("should throw");
  }
  catch (const std::runtime_error&)
  {
    /* expected */
  }
  try
  {
    c.value (key_a_b);
    BOOST_FAIL ("should throw");
  }
  catch (const std::runtime_error&)
  {
    /* expected */
  }
}
