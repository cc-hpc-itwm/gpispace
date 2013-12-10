// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_require_type
#include <boost/test/unit_test.hpp>

#include <we/require_type.hpp>

#include <we/type/value/poke.hpp>
#include <we/exception.hpp>

BOOST_AUTO_TEST_CASE (require_type)
{
  using pnet::type::value::value_type;
  using pnet::type::signature::signature_type;
  using pnet::type::signature::structured_type;
  using pnet::type::signature::structure_type;
  using pnet::type::value::poke;

#define OKAY(_s,_v...)                                                  \
  BOOST_CHECK_NO_THROW ( pnet::require_type ( value_type (_v)           \
                                            , signature_type (_s)       \
                                            )                           \
                       )
#define BAD(_e,_s,_v...)                                        \
  BOOST_CHECK_THROW ( pnet::require_type ( value_type (_v)      \
                                         , signature_type (_s)  \
                                         )                      \
                    , pnet::exception::_e                       \
                    )

#define OKAY_LITERAL(_s,_v...) OKAY(std::string (_s), _v)

  OKAY_LITERAL ("control", we::type::literal::control());
  OKAY_LITERAL ("bool", true);
  OKAY_LITERAL ("int", 23);
  OKAY_LITERAL ("long", 23L);
  OKAY_LITERAL ("unsigned int", 23U);
  OKAY_LITERAL ("unsigned long", 23UL);
  OKAY_LITERAL ("float", 0.0f);
  OKAY_LITERAL ("double", 0.0);
  OKAY_LITERAL ("char", 'c');
  OKAY_LITERAL ("string", std::string (""));
  OKAY_LITERAL ("bitset", bitsetofint::type());
  OKAY_LITERAL ("bytearray", bytearray::type());
  OKAY_LITERAL ("list", std::list<value_type>());
  OKAY_LITERAL ("set", std::set<value_type>());
  OKAY_LITERAL ("map", std::map<value_type, value_type>());

  BAD (type_mismatch, std::string ("float"), 23);

  BAD ( type_mismatch
      , structured_type (std::make_pair ("s", structure_type()))
      , 23
      );

  OKAY ( structured_type (std::make_pair ("s", structure_type()))
       , pnet::type::value::structured_type()
       );

  BAD ( type_mismatch
      , std::string ("int")
      , pnet::type::value::structured_type()
      );

  {
    value_type v;
    poke ("a", v, 23);

    BAD ( unknown_field
        , structured_type (std::make_pair ("s", structure_type()))
        , v
        );

    {
      structure_type s;
      s.push_back (std::make_pair (std::string ("s"), std::string ("a")));

      BAD ( unknown_field
          , structured_type (std::make_pair ("s", s))
          , v
          );
    }

    {
      structure_type s;
      s.push_back (std::make_pair (std::string ("s"), std::string ("int")));

      BAD ( unknown_field
          , structured_type (std::make_pair ("s", s))
          , v
          );
    }

    {
      structure_type s;
      s.push_back (std::make_pair (std::string ("a"), std::string ("float")));

      BAD ( type_mismatch
          , structured_type (std::make_pair ("s", s))
          , v
          );
    }

    {
      structure_type s;
      s.push_back (std::make_pair (std::string ("a"), std::string ("int")));

      OKAY ( structured_type (std::make_pair ("s", s))
           , v
           );
    }

    {
      structure_type s;
      s.push_back (std::make_pair (std::string ("a"), std::string ("int")));
      s.push_back (std::make_pair (std::string ("b"), std::string ("int")));

      BAD ( missing_field
          , structured_type (std::make_pair ("s", s))
          , v
          );
    }
  }

  {
    value_type p1;
    poke ("x", p1, 0.0f);
    poke ("y", p1, 1.0f);
    value_type p2;
    poke ("x", p2, 2.0f);
    poke ("y", p2, 3.0f);
    value_type l;
    poke ("p", l, p1);
    poke ("q", l, p2);

#define FIELD(_name,_type)                                      \
    std::make_pair (std::string (_name), std::string (_type))
#define STRUCT(_name,_type)                                     \
    structured_type (std::make_pair (std::string (_name), _type))

    structure_type point2D;
    point2D.push_back (FIELD ("x", "float"));
    point2D.push_back (FIELD ("y", "float"));
    structure_type line2D;
    line2D.push_back (STRUCT ("p", point2D));
    line2D.push_back (STRUCT ("q", point2D));

    OKAY (STRUCT ("line2D", line2D), l);

#undef STRUCT
#undef FIELD
  }

#undef BAD
#undef OKAY_LITERAL
#undef OKAY
}
