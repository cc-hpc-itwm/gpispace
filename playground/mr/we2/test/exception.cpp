// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_exception
#include <boost/test/unit_test.hpp>

#include <we2/exception.hpp>

#include <we2/type/value/poke.hpp>
#include <we2/signature_of.hpp>

namespace
{
  struct fix
  {
    std::list<std::string> path() const
    {
      std::list<std::string> path;
      path.push_back ("p");
      path.push_back ("p");

      return path;
    }

    pnet::type::value::value_type value() const
    {
      pnet::type::value::value_type v;
      pnet::type::value::poke ("e", v, 42UL);
      pnet::type::value::poke ("c", v, 'c');

      return v;
    }

    pnet::type::signature::signature_type signature() const
    {
      return pnet::signature_of (value());
    }
  };
}

BOOST_FIXTURE_TEST_CASE (type_mismatch, fix)
{
  const  pnet::exception::type_mismatch e (signature(), value(), path());

  BOOST_CHECK (e.signature() == signature());
  BOOST_CHECK (e.value() == value());
  BOOST_CHECK (e.path() == path());
  BOOST_CHECK (e.what() == std::string ("type error: type mismatch for field 'p.p': expected type 'struct :: [e :: unsigned long, c :: char]', value 'struct [e := 42UL, c := 'c']' has type 'struct :: [e :: unsigned long, c :: char]'"));
}

BOOST_FIXTURE_TEST_CASE (missing_field, fix)
{
  const pnet::exception::missing_field e (signature(), path());

  BOOST_CHECK (e.signature() == signature());
  BOOST_CHECK (e.path() == path());
  BOOST_CHECK (e.what() == std::string ("type error: missing field 'p.p' of type 'struct :: [e :: unsigned long, c :: char]'"));
}

BOOST_FIXTURE_TEST_CASE (unknown_field, fix)
{
  const pnet::exception::unknown_field e (value(), path());

  BOOST_CHECK (e.value() == value());
  BOOST_CHECK (e.path() == path());
  BOOST_CHECK (e.what() == std::string ("type error: unknown field 'p.p' with value 'struct [e := 42UL, c := 'c']' of type 'struct :: [e :: unsigned long, c :: char]'"));
}
