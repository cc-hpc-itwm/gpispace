// Copyright (C) 2013-2016,2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/we/require_type.hpp>

#include <gspc/we/exception.hpp>
#include <gspc/we/type/shared.hpp>
#include <gspc/we/type/signature/contains_shared.hpp>
#include <gspc/we/type/value/poke.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (require_type)
{
  using gspc::pnet::type::value::value_type;
  using gspc::pnet::type::signature::signature_type;
  using gspc::pnet::type::signature::structured_type;
  using gspc::pnet::type::signature::structure_type;
  using gspc::pnet::type::value::poke;

#define OKAY(_s,_v...)                                                  \
  BOOST_CHECK_NO_THROW ( gspc::pnet::require_type ( value_type (_v)           \
                                            , signature_type (_s)       \
                                            )                           \
                       )
#define BAD(_e,_s,_v...)                                        \
  BOOST_CHECK_THROW ( gspc::pnet::require_type ( value_type (_v)      \
                                         , signature_type (_s)  \
                                         )                      \
                    , gspc::pnet::exception::_e                       \
                    )

#define OKAY_LITERAL(_s,_v...) OKAY(std::string (_s), _v)

  OKAY_LITERAL ("control", gspc::we::type::literal::control());
  OKAY_LITERAL ("bool", true);
  OKAY_LITERAL ("int", 23);
  OKAY_LITERAL ("long", 23L);
  OKAY_LITERAL ("unsigned int", 23U);
  OKAY_LITERAL ("unsigned long", 23UL);
  OKAY_LITERAL ("float", 0.0f);
  OKAY_LITERAL ("double", 0.0);
  OKAY_LITERAL ("char", 'c');
  OKAY_LITERAL ("string", std::string (""));
  OKAY_LITERAL ("bitset", gspc::pnet::type::bitsetofint::type());
  OKAY_LITERAL ("bytearray", gspc::we::type::bytearray());
  OKAY_LITERAL ("list", std::list<value_type>());
  OKAY_LITERAL ("set", std::set<value_type>());
  OKAY_LITERAL ("map", std::map<value_type, value_type>());

  BAD (type_mismatch, std::string ("float"), 23);

  BAD ( type_mismatch
      , structured_type (std::make_pair ("s", structure_type()))
      , 23
      );

  OKAY ( structured_type (std::make_pair ("s", structure_type()))
       , gspc::pnet::type::value::structured_type()
       );

  BAD ( type_mismatch
      , std::string ("int")
      , gspc::pnet::type::value::structured_type()
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

// Test that shared type checks cleanup place name at runtime
BOOST_AUTO_TEST_CASE (shared_type_checks_cleanup_place_name)
{
  using gspc::pnet::type::value::value_type;
  using gspc::pnet::type::signature::signature_type;

  // A shared value with cleanup place "cleanup"
  gspc::we::type::shared const shared_value {std::string {"test"}, "cleanup"};
  value_type const v {shared_value};

  // Type matches: shared_cleanup matches shared with cleanup place "cleanup"
  BOOST_CHECK_NO_THROW
    ( gspc::pnet::require_type (v, signature_type {std::string {"shared_cleanup"}})
    );

  // Type mismatch: shared_other does not match shared with cleanup place "cleanup"
  BOOST_CHECK_THROW
    ( gspc::pnet::require_type (v, signature_type {std::string {"shared_other"}})
    , gspc::pnet::exception::type_mismatch
    );

  // Type mismatch: non-shared type does not match shared value
  BOOST_CHECK_THROW
    ( gspc::pnet::require_type (v, signature_type {std::string {"string"}})
    , gspc::pnet::exception::type_mismatch
    );
}

// Test that wrapped value type is NOT checked when placing a shared value
// on a shared place - but WILL be checked when the cleanup token is placed
// on the cleanup place. This test demonstrates that require_type for a shared
// value only checks the cleanup place name, not the wrapped value type.
BOOST_AUTO_TEST_CASE (shared_wrapped_value_type_not_checked_on_shared_place)
{
  using gspc::pnet::type::value::value_type;
  using gspc::pnet::type::signature::signature_type;

  // A shared with a long value (not an int)
  gspc::we::type::shared const shared_long {42L, "cleanup"};
  value_type const v {shared_long};

  // Placing on a shared_cleanup place only checks the cleanup place name
  // The wrapped value type (long) is NOT checked here
  BOOST_CHECK_NO_THROW
    ( gspc::pnet::require_type (v, signature_type {std::string {"shared_cleanup"}})
    );

  // However, when the cleanup happens and the wrapped value (42L, a long)
  // needs to be placed on an int cleanup place, it will fail.
  // This simulates what happens when the cleanup token is placed:
  value_type const wrapped_value {42L};  // This is a long

  // Placing a long on an int place fails
  BOOST_CHECK_THROW
    ( gspc::pnet::require_type (wrapped_value, signature_type {std::string {"int"}})
    , gspc::pnet::exception::type_mismatch
    );
}

// Tests for signature_contains_shared utility function
namespace gspc::pnet::type::signature
{
  BOOST_AUTO_TEST_CASE (primitive_types_does_not_contain_shared)
  {
    BOOST_CHECK (!contains_shared (std::string {"int"}));
    BOOST_CHECK (!contains_shared (std::string {"long"}));
    BOOST_CHECK (!contains_shared (std::string {"string"}));
    BOOST_CHECK (!contains_shared (std::string {"list"}));
    BOOST_CHECK (!contains_shared (std::string {"control"}));
  }

  BOOST_AUTO_TEST_CASE (shared_contains_shared)
  {
    BOOST_CHECK (contains_shared (std::string {"shared_cleanup"}));
    BOOST_CHECK (contains_shared (std::string {"shared_some_place"}));
  }

  BOOST_AUTO_TEST_CASE (struct_without_shared_does_not_contain_shared)
  {
    auto const struct_without_shared
      { structure_type
        { std::make_pair (std::string {"x"}, std::string {"int"})
        , std::make_pair (std::string {"y"}, std::string {"long"})
        }
      };
    BOOST_CHECK
      ( !contains_shared (structured_type {"point", struct_without_shared})
      );
  }

  BOOST_AUTO_TEST_CASE (struct_with_shared_contains_shared)
  {
    auto const struct_with_shared
      { structure_type
        { std::make_pair (std::string {"data"}, std::string {"int"})
        , std::make_pair (std::string {"ref"}, std::string {"shared_cleanup"})
        }
      };
    BOOST_CHECK
      ( contains_shared (structured_type {"data_ref", struct_with_shared})
      );
  }

  BOOST_AUTO_TEST_CASE (struct_with_nested_shared_contains_shared)
  {
    // Inner struct with shared
    auto const inner
      { structure_type
        { std::make_pair (std::string {"value"}, std::string {"shared_cleanup"})
        }
      };

    // Outer struct containing inner struct
    auto const outer
      { structure_type
        { std::make_pair (std::string {"x"}, std::string {"int"})
        , std::make_pair (std::string {"inner"}, inner)
        }
      };

    BOOST_CHECK
      ( contains_shared (structured_type {"nested", outer})
      );
  }
}
