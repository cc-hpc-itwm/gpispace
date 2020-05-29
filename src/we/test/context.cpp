#include <boost/test/unit_test.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <we/expr/eval/context.hpp>

#include <we/type/value.hpp>
#include <we/type/value/read.hpp>

#include <we/type/value/boost/test/printer.hpp>

BOOST_AUTO_TEST_CASE (basic)
{
  typedef expr::eval::context context_t;
  typedef pnet::type::value::value_type value_type;

  using pnet::type::value::read;

  context_t c;

  value_type x (read ("0L"));
  value_type y (read ("[]"));
  value_type z (read ("Struct [x:=0.0,y:=\"a_string\"]"));
  value_type z_x (read ("0.0"));
  value_type z_y (read ("\"a_string\""));

  std::list<std::string> key_z_x;
  key_z_x.push_back ("z");
  key_z_x.push_back ("x");

  std::list<std::string> key_z_y;
  key_z_y.push_back ("z");
  key_z_y.push_back ("y");

  std::list<std::string> key_z_a;
  key_z_a.push_back ("z");
  key_z_a.push_back ("a");

  std::list<std::string> key_a_b;
  key_a_b.push_back ("a");
  key_a_b.push_back ("b");

  c.bind_and_discard_ref ({"x"}, x);
  c.bind_and_discard_ref ({"y"}, y);
  c.bind_and_discard_ref ({"z"}, z);

  BOOST_REQUIRE_EQUAL (c.value ({"x"}), x);
  BOOST_REQUIRE_EQUAL (c.value ({"y"}), y);
  BOOST_REQUIRE_EQUAL (c.value ({"z"}), z);
  BOOST_REQUIRE_EQUAL (c.value (key_z_x), z_x);
  BOOST_REQUIRE_EQUAL (c.value (key_z_y), z_y);

  BOOST_REQUIRE_THROW (c.value (key_z_a), std::runtime_error);
  BOOST_REQUIRE_THROW (c.value (key_a_b), std::runtime_error);

  {
    const double z_x_new (1.0);

    c.bind_and_discard_ref ({"z","x"}, pnet::type::value::value_type (z_x_new));

    BOOST_REQUIRE_EQUAL (c.value (key_z_x), value_type (z_x_new));
  }

  {
    const std::string z_new ("z");

    c.bind_and_discard_ref ({"z"}, pnet::type::value::value_type (z_new));

    BOOST_REQUIRE_EQUAL (c.value ({"z"}), value_type (z_new));
  }

  BOOST_REQUIRE_EQUAL (c.value ({"x"}), x);
  BOOST_REQUIRE_EQUAL (c.value ({"y"}), y);

  BOOST_REQUIRE_THROW (c.value (key_z_a), std::runtime_error);
  BOOST_REQUIRE_THROW (c.value (key_a_b), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (reference)
{
  typedef expr::eval::context context_t;
  typedef pnet::type::value::value_type value_type;

  using pnet::type::value::read;

  context_t c;

  const std::string key_a ("a");
  const std::string key_b ("b");
  const std::string key_c ("c");

  const value_type value_a (0.0);
  const value_type value_b = std::string("a_string");
  const value_type value_c (read ("Struct [x:=1L,y:=Struct [a:=2L,b:=3L]]"));

  c.bind_ref (key_a, value_a);
  c.bind_ref (key_b, value_b);
  c.bind_ref (key_c, value_c);

  BOOST_REQUIRE_EQUAL (c.value ({key_a}), value_a);
  BOOST_REQUIRE_EQUAL (c.value ({key_b}), value_b);
  BOOST_REQUIRE_EQUAL (c.value ({key_c}), value_c);

  std::list<std::string> key_c_x;
  key_c_x.push_back ("c");
  key_c_x.push_back ("x");
  std::list<std::string> key_c_y_a;
  key_c_y_a.push_back ("c");
  key_c_y_a.push_back ("y");
  key_c_y_a.push_back ("a");
  std::list<std::string> key_c_y_b;
  key_c_y_b.push_back ("c");
  key_c_y_b.push_back ("y");
  key_c_y_b.push_back ("b");

  BOOST_REQUIRE_EQUAL (c.value (key_c_x), value_type (1L));
  BOOST_REQUIRE_EQUAL (c.value (key_c_y_a), value_type (2L));
  BOOST_REQUIRE_EQUAL (c.value (key_c_y_b), value_type (3L));
}
