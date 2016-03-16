// mirko.rahn@itwm.fraunhofer.de

#include <boost/test/unit_test.hpp>

#include <xml/parse/type/memory_transfer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/join.hpp>
#include <util-generic/testing/random_string.hpp>
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
                  , boost::optional<bool> not_modified_in_module_call
                  )
  {
    std::list<std::string> expressions_global;

    while (num_expressions_global --> 0)
    {
      expressions_global.push_back (fhg::util::testing::random_string_without_zero());
    }

    std::list<std::string> expressions_local;

    while (num_expressions_local --> 0)
    {
      expressions_local.push_back (fhg::util::testing::random_string_without_zero());
    }

    std::ostringstream oss;

    fhg::util::xml::xmlstream s (oss);

    xml::parse::type::dump::dump
      (s, make_transfer ( fhg::util::join (expressions_global, ';').string()
                        , fhg::util::join (expressions_local, ';').string()
                        )
      );

    const std::string expected
      ( ( boost::format (R"EOS(<memory-%1%%4%>
  <global>%2%</global>
  <local>%3%</local>
</memory-%1%>)EOS")
        % tag
        % fhg::util::join (expressions_global, ';')
        % fhg::util::join (expressions_local, ';')
        % ( not_modified_in_module_call
          ? ( boost::format (" not-modified-in-module-call=\"%1%\"")
            % (*not_modified_in_module_call ? "true" : "false")
            ).str()
          : ""
          )
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
            (nullptr, nullptr, fhg::util::testing::random_string())
          , expressions_global
          , expressions_local
          , we::type::property::type()
          );
      }
      , boost::none
      );
  }

  void check_dump_put ( std::size_t num_expressions_global
                      , std::size_t num_expressions_local
                      , boost::optional<bool> not_modified_in_module_call
                      )
  {
    check_dump<xml::parse::type::memory_put>
      ( num_expressions_global
      , num_expressions_local
      , "put"
      , [&not_modified_in_module_call]
          ( std::string const& expressions_global
          , std::string const& expressions_local
          )
      { return xml::parse::type::memory_put
          ( xml::parse::util::position_type
            (nullptr, nullptr, fhg::util::testing::random_string())
          , expressions_global
          , expressions_local
          , we::type::property::type()
          , not_modified_in_module_call
          );
      }
      , not_modified_in_module_call
      );
  }

  void check_dump_getput ( std::size_t num_expressions_global
                         , std::size_t num_expressions_local
                         , boost::optional<bool> not_modified_in_module_call
                         )
  {
    check_dump<xml::parse::type::memory_getput>
      ( num_expressions_global
      , num_expressions_local
      , "getput"
      , [&not_modified_in_module_call]
          ( std::string const& expressions_global
          , std::string const& expressions_local
          )
      { return xml::parse::type::memory_getput
          ( xml::parse::util::position_type
            (nullptr, nullptr, fhg::util::testing::random_string())
          , expressions_global
          , expressions_local
          , we::type::property::type()
          , not_modified_in_module_call
          );
      }
      , not_modified_in_module_call
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
      check_dump_put (global, local, boost::none);
      check_dump_put (global, local, true);
      check_dump_put (global, local, false);
      check_dump_getput (global, local, boost::none);
      check_dump_getput (global, local, true);
      check_dump_getput (global, local, false);
    }
  }
}
