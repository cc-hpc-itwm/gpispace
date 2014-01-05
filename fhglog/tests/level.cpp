// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE fhglog_level
#include <boost/test/unit_test.hpp>

#include <fhglog/level.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>

BOOST_AUTO_TEST_CASE (from_int)
{
#define OKAY(_l, _i)                                            \
  BOOST_REQUIRE_EQUAL (fhg::log::_l, fhg::log::from_int (_i))

  OKAY (TRACE, 0);
  OKAY (DEBUG, 1);
  OKAY (INFO, 2);
  OKAY (WARN, 3);
  OKAY (ERROR, 4);
  OKAY (FATAL, 5);

#undef OKAY

  BOOST_REQUIRE_THROW (fhg::log::from_int (-1), std::runtime_error);
  BOOST_REQUIRE_THROW (fhg::log::from_int (6), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (from_string)
{
#define OKAY(_l, _s)                                                    \
  BOOST_REQUIRE_EQUAL (fhg::log::_l, fhg::log::from_string (_s))

  OKAY (TRACE, "TRACE");
  OKAY (DEBUG, "DEBUG");
  OKAY (INFO, "INFO");
  OKAY (WARN, "WARN");
  OKAY (ERROR, "ERROR");
  OKAY (FATAL, "FATAL");

#undef OKAY

  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&fhg::log::from_string, "TRACEmore")
    , "PARSE ERROR [5]: additional input\n"
      "TRACE more\n"
      "     ^\n"
    );
  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&fhg::log::from_string, "DEBUGmore")
    , "PARSE ERROR [5]: additional input\n"
      "DEBUG more\n"
      "     ^\n"
    );
  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&fhg::log::from_string, "INFOmore")
    , "PARSE ERROR [4]: additional input\n"
      "INFO more\n"
      "    ^\n"
    );
  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&fhg::log::from_string, "WARNmore")
    , "PARSE ERROR [4]: additional input\n"
      "WARN more\n"
      "    ^\n"
    );
  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&fhg::log::from_string, "ERRORmore")
    , "PARSE ERROR [5]: additional input\n"
      "ERROR more\n"
      "     ^\n"
    );
  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&fhg::log::from_string, "FATALmore")
    , "PARSE ERROR [5]: additional input\n"
      "FATAL more\n"
      "     ^\n"
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
  THROW ("fatal");

#undef THROW
}

BOOST_AUTO_TEST_CASE (string)
{
#define OKAY(_l, _s)                                       \
  BOOST_REQUIRE_EQUAL ( fhg::log::string (fhg::log::_l)    \
                      , std::string (_s)                   \
                      )

  OKAY (TRACE, "TRACE");
  OKAY (DEBUG, "DEBUG");
  OKAY (INFO, "INFO");
  OKAY (WARN, "WARN");
  OKAY (ERROR, "ERROR");
  OKAY (FATAL, "FATAL");

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
  OKAY (DEBUG);
  OKAY (INFO);
  OKAY (WARN);
  OKAY (ERROR);
  OKAY (FATAL);

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
  OKAY ("DEBUG");
  OKAY ("INFO");
  OKAY ("WARN");
  OKAY ("ERROR");
  OKAY ("FATAL");

#undef OKAY
}
