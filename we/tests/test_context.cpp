// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE context
#include <boost/test/unit_test.hpp>

#include <we/expr/eval/context.hpp>

#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

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

  BOOST_REQUIRE_EQUAL (c.value ("x"), x);
  BOOST_REQUIRE_EQUAL (c.value ("y"), y);
  BOOST_REQUIRE_EQUAL (c.value ("z"), z);
  BOOST_REQUIRE_EQUAL (c.value (key_z_x), z_x);
  BOOST_REQUIRE_EQUAL (c.value (key_z_y), z_y);

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

    BOOST_REQUIRE_EQUAL (c.value (key_z_x), value::type (z_x_new));
  }

  {
    const std::string z_new ("z");

    c.bind ("z", z_new);

    BOOST_REQUIRE_EQUAL (c.value ("z"), value::type (z_new));
  }

  BOOST_REQUIRE_EQUAL (c.value ("x"), x);
  BOOST_REQUIRE_EQUAL (c.value ("y"), y);

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

BOOST_AUTO_TEST_CASE (reference)
{
  typedef expr::eval::context context_t;
  typedef context_t::key_vec_t key_vec_t;

  context_t c;

  const std::string key_a ("a");
  const std::string key_b ("b");
  const std::string key_c ("c");

  const value::type value_a (0.0);
  const value::type value_b ("a_string");
  const value::type value_c (value::read ("[x:=1,y:=[a:=2,b:=3]]"));

  c.bind_ref (key_a, value_a);
  c.bind_ref (key_b, value_b);
  c.bind_ref (key_c, value_c);

  BOOST_REQUIRE_EQUAL (c.value (key_a), value_a);
  BOOST_REQUIRE_EQUAL (c.value (key_b), value_b);
  BOOST_REQUIRE_EQUAL (c.value (key_c), value_c);

  key_vec_t key_c_x;
  key_c_x.push_back ("c");
  key_c_x.push_back ("x");
  key_vec_t key_c_y_a;
  key_c_y_a.push_back ("c");
  key_c_y_a.push_back ("y");
  key_c_y_a.push_back ("a");
  key_vec_t key_c_y_b;
  key_c_y_b.push_back ("c");
  key_c_y_b.push_back ("y");
  key_c_y_b.push_back ("b");

  BOOST_REQUIRE_EQUAL (c.value (key_c_x), value::type (1L));
  BOOST_REQUIRE_EQUAL (c.value (key_c_y_a), value::type (2L));
  BOOST_REQUIRE_EQUAL (c.value (key_c_y_b), value::type (3L));
}
