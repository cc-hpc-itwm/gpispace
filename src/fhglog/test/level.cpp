// alexander.petry@itwm.fraunhofer.de

#include <boost/test/unit_test.hpp>

#include <fhglog/level.hpp>
#include <fhglog/level_io.hpp>

#include <fhg/util/parse/error.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

BOOST_AUTO_TEST_CASE (from_string)
{
#define OKAY(_l, _s)                                                    \
  BOOST_REQUIRE_EQUAL (fhg::log::_l, fhg::log::from_string (_s))

  OKAY (TRACE, "TRACE");
  OKAY (INFO, "INFO");
  OKAY (WARN, "WARN");
  OKAY (ERROR, "ERROR");

#undef OKAY

  fhg::util::testing::require_exception
    ( [] { fhg::log::from_string ("TRACEmore"); }
    , std::runtime_error ( "PARSE ERROR [5]: additional input\n"
                           "TRACE more\n"
                           "     ^\n"
                         )
    );
  fhg::util::testing::require_exception
    ( [] { fhg::log::from_string ("INFOmore"); }
    , std::runtime_error ( "PARSE ERROR [4]: additional input\n"
                           "INFO more\n"
                           "    ^\n"
                         )
    );
  fhg::util::testing::require_exception
    ( [] { fhg::log::from_string ("WARNmore"); }
    , std::runtime_error ( "PARSE ERROR [4]: additional input\n"
                           "WARN more\n"
                           "    ^\n"
                         )
    );
  fhg::util::testing::require_exception
    ( [] { fhg::log::from_string ("ERRORmore"); }
    , std::runtime_error ( "PARSE ERROR [5]: additional input\n"
                           "ERROR more\n"
                           "     ^\n"
                         )
    );

#define THROW(_s)                                                \
  BOOST_REQUIRE_THROW ( fhg::log::from_string (_s)               \
                      , fhg::util::parse::error::expected        \
                      )

  THROW ("");
  THROW ("T");
  THROW ("D");
  THROW ("I");
  THROW ("W");
  THROW ("E");
  THROW ("F");
  THROW ("trace");
  THROW ("debug");
  THROW ("info");
  THROW ("warn");
  THROW ("error");

#undef THROW
}

BOOST_AUTO_TEST_CASE (string)
{
#define OKAY(_l, _s)                                       \
  BOOST_REQUIRE_EQUAL ( fhg::log::string (fhg::log::_l)    \
                      , std::string (_s)                   \
                      )

  OKAY (TRACE, "TRACE");
  OKAY (INFO, "INFO");
  OKAY (WARN, "WARN");
  OKAY (ERROR, "ERROR");

#undef OKAY

  BOOST_REQUIRE_THROW ( fhg::log::string ((fhg::log::Level)-1)
                      , std::runtime_error
                      );
  BOOST_REQUIRE_THROW ( fhg::log::string ((fhg::log::Level)6)
                      , std::runtime_error
                      );
}

BOOST_AUTO_TEST_CASE (from_string_string_id)
{
#define OKAY(_l)                                              \
  BOOST_REQUIRE_EQUAL                                         \
    ( fhg::log::_l                                            \
    , fhg::log::from_string (fhg::log::string (fhg::log::_l)) \
    )

  OKAY (TRACE);
  OKAY (INFO);
  OKAY (WARN);
  OKAY (ERROR);

#undef OKAY
}

BOOST_AUTO_TEST_CASE (string_from_string_id)
{
#define OKAY(_s)                                              \
  BOOST_REQUIRE_EQUAL                                         \
    ( std::string (_s)                                        \
    , fhg::log::string (fhg::log::from_string (_s))           \
    )

  OKAY ("TRACE");
  OKAY ("INFO");
  OKAY ("WARN");
  OKAY ("ERROR");

#undef OKAY
}
