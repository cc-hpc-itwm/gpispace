// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_transition
#include <boost/test/unit_test.hpp>

#include <we/type/transition.hpp>

#include <we/type/property.hpp>
//! \todo: eliminate this include that just completes the type
#include <we/type/net.hpp>

#include <fhg/util/random_string.hpp>

namespace
{
  typedef  std::vector<std::pair< pnet::type::value::value_type
                                , we::port_id_type
                                >
                      > input_t;

  std::string random_identifier()
  {
    return
      fhg::util::random_char_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_")
      +
      fhg::util::random_string_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
  }
}

BOOST_AUTO_TEST_CASE (get_schedule_data_not_set)
{
  we::type::transition_t const transition
    ( fhg::util::random_string()
    , we::type::expression_t()
    , boost::none
    , bool()
    , we::type::property::type()
    , we::priority_type()
    );

  BOOST_REQUIRE (!transition.get_schedule_data<unsigned long>
                  (input_t(), "num_worker")
                );
  BOOST_REQUIRE (!transition.get_schedule_data<unsigned long>
                  (input_t(), fhg::util::random_string())
                );
}

BOOST_AUTO_TEST_CASE (get_schedule_data_constant_string)
{
  std::string const key (fhg::util::random_string());
  std::string const value (fhg::util::random_string_without ("\\\""));

  we::type::property::path_type path;
  path.push_back ("fhg");
  path.push_back ("drts");
  path.push_back ("schedule");
  path.push_back (key);

  we::type::property::type properties;
  properties.set (path, "\"" + value + "\"");

  we::type::transition_t const transition
    ( fhg::util::random_string()
    , we::type::expression_t()
    , boost::none
    , bool()
    , properties
    , we::priority_type()
    );

  BOOST_REQUIRE_EQUAL
    ( transition.get_schedule_data<std::string> (input_t(), key).get()
    , value
    );
}

BOOST_AUTO_TEST_CASE (get_schedule_data_constant_long)
{
  std::string const key (fhg::util::random_string());
  unsigned long const value (rand());

  we::type::property::path_type path;
  path.push_back ("fhg");
  path.push_back ("drts");
  path.push_back ("schedule");
  path.push_back (key);

  we::type::property::type properties;
  properties.set (path, boost::lexical_cast<std::string> (value) + "UL");

  we::type::transition_t const transition
    ( fhg::util::random_string()
    , we::type::expression_t()
    , boost::none
    , bool()
    , properties
    , we::priority_type()
    );

  BOOST_REQUIRE_EQUAL
    ( transition.get_schedule_data<unsigned long> (input_t(), key).get()
    , value
    );
}

BOOST_AUTO_TEST_CASE (get_schedule_data_expression_simple)
{
  std::string const key (fhg::util::random_string());
  std::string const port_name (random_identifier());
  unsigned long const value (rand());

  we::type::property::path_type path;
  path.push_back ("fhg");
  path.push_back ("drts");
  path.push_back ("schedule");
  path.push_back (key);

  we::type::property::type properties;
  properties.set (path, "${" + port_name + "}");

  we::type::transition_t transition
    ( fhg::util::random_string()
    , we::type::expression_t()
    , boost::none
    , bool()
    , properties
    , we::priority_type()
    );

  we::port_id_type const port_id
    ( transition.add_port
      ( we::type::port_t
        ( port_name
        , we::type::PORT_IN
        , pnet::type::signature::signature_type (std::string ("unsigned long"))
        , we::type::property::type()
        )
      )
    );

  input_t input;
  input.push_back (std::make_pair (value, port_id));

  BOOST_REQUIRE_EQUAL
    ( transition.get_schedule_data<unsigned long> (input, key).get()
    , value
    );
}

BOOST_AUTO_TEST_CASE (get_schedule_data_expression_sum)
{
  std::string const key (fhg::util::random_string());
  std::string const port_name1 (random_identifier());
  std::string const port_name2 (random_identifier());
  unsigned long const value1 (rand());
  unsigned long const value2 (rand());

  we::type::property::path_type path;
  path.push_back ("fhg");
  path.push_back ("drts");
  path.push_back ("schedule");
  path.push_back (key);

  we::type::property::type properties;
  properties.set (path, "${" + port_name1 + "} + ${" + port_name2 + "}");

  we::type::transition_t transition
    ( fhg::util::random_string()
    , we::type::expression_t()
    , boost::none
    , bool()
    , properties
    , we::priority_type()
    );

  we::port_id_type const port_id1
    ( transition.add_port
      ( we::type::port_t
        ( port_name1
        , we::type::PORT_IN
        , pnet::type::signature::signature_type (std::string ("unsigned long"))
        , we::type::property::type()
        )
      )
    );
  we::port_id_type const port_id2
    ( transition.add_port
      ( we::type::port_t
        ( port_name2
        , we::type::PORT_IN
        , pnet::type::signature::signature_type (std::string ("unsigned long"))
        , we::type::property::type()
        )
      )
    );

  input_t input;
  input.push_back (std::make_pair (value1, port_id1));
  input.push_back (std::make_pair (value2, port_id2));

  BOOST_REQUIRE_EQUAL
    ( transition.get_schedule_data<unsigned long> (input, key).get()
    , value1 + value2
    );
}