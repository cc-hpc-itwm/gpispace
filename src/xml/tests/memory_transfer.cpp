// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE memory_transfer
#include <boost/test/unit_test.hpp>

#include <xml/parse/type/memory_transfer.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/random_string.hpp>
#include <fhg/util/xml.hpp>

#include <boost/format.hpp>

#include <iomanip>
#include <functional>

namespace
{
  template<typename Transfer>
  void check_dump ( std::size_t num_expressions_global
                  , std::size_t num_expressions_local
                  , std::string const& tag
                  , std::function<Transfer ( std::string const&
                                           , std::string const&
                                           )> make_transfer
                  )
  {
    std::list<std::string> expressions_global;

    while (num_expressions_global --> 0)
    {
      expressions_global.push_back (fhg::util::random_string_without_zero());
    }

    std::list<std::string> expressions_local;

    while (num_expressions_local --> 0)
    {
      expressions_local.push_back (fhg::util::random_string_without_zero());
    }

    std::ostringstream oss;

    fhg::util::xml::xmlstream s (oss);

    xml::parse::type::dump::dump
      (s, make_transfer ( fhg::util::join (expressions_global, ";")
                        , fhg::util::join (expressions_local, ";")
                        )
      );

    const std::string expected
      ( ( boost::format (R"EOS(<memory-%1%>
  <global>%2%</global>
  <local>%3%</local>
</memory-%1%>)EOS")
        % tag
        % fhg::util::join (expressions_global, ";")
        % fhg::util::join (expressions_local, ";")
        ).str()
      );

    BOOST_REQUIRE_EQUAL (expected, oss.str());
  }

  void check_dump_get ( std::size_t num_expressions_global
                      , std::size_t num_expressions_local
                      )
  {
    check_dump<xml::parse::type::memory_get>
      ( num_expressions_global
      , num_expressions_local
      , "get"
      , []( std::string const& expressions_global
          , std::string const& expressions_local
          )
      { return xml::parse::type::memory_get
          ( xml::parse::util::position_type
            (nullptr, nullptr, fhg::util::random_string())
          , expressions_global
          , expressions_local
          , we::type::property::type()
          );
      }
      );
  }

  void check_dump_put ( std::size_t num_expressions_global
                      , std::size_t num_expressions_local
                      )
  {
    check_dump<xml::parse::type::memory_put>
      ( num_expressions_global
      , num_expressions_local
      , "put"
      , []( std::string const& expressions_global
          , std::string const& expressions_local
          )
      { return xml::parse::type::memory_put
          ( xml::parse::util::position_type
            (nullptr, nullptr, fhg::util::random_string())
          , expressions_global
          , expressions_local
          , we::type::property::type()
          );
      }
      );
  }

  void check_dump_getput ( std::size_t num_expressions_global
                         , std::size_t num_expressions_local
                         )
  {
    check_dump<xml::parse::type::memory_getput>
      ( num_expressions_global
      , num_expressions_local
      , "getput"
      , []( std::string const& expressions_global
          , std::string const& expressions_local
          )
      { return xml::parse::type::memory_getput
          ( xml::parse::util::position_type
            (nullptr, nullptr, fhg::util::random_string())
          , expressions_global
          , expressions_local
          , we::type::property::type()
          );
      }
      );
  }
}

BOOST_AUTO_TEST_CASE (dump_get)
{
  for (std::size_t global (0); global < 4; ++global)
  {
    for (std::size_t local (0); local < 4; ++local)
    {
      check_dump_get (global, local);
      check_dump_put (global, local);
      check_dump_getput (global, local);
    }
  }
}
