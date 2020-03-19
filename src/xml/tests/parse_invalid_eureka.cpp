#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <parser_fixture.hpp>
#include <xml/parse/error.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>

BOOST_FIXTURE_TEST_CASE ( duplicate_eureka_connections
                        , parser_fixture
                        )
{
  BOOST_REQUIRE_THROW
    ( parse ("connect_eureka_duplicate_port.xpnet")
    , ::xml::parse::error::duplicate_eureka
    );
}

BOOST_AUTO_TEST_CASE (no_output_port_for_eureka)
{
  std::string const trans_name (fhg::util::testing::random_identifier());
  std::string const port_name (fhg::util::testing::random_identifier());

  std::string const input
    ( ( boost::format (R"EOS(
<defun name="foo">
 <net>
  <place name="P" type="set"/>
  <transition name="%1%">
    <defun>
      <in name="A" type="set"/>
      <expression/>
    </defun>
    <connect-in port="A" place="P"/>
    <connect-eureka port="%2%"/>
  </transition>
 </net>
</defun>)EOS")
      % trans_name
      % port_name
      ).str()
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::connect_eureka_to_nonexistent_out_port>
    ( [&input, &port_name, &trans_name]()
      { xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (xml::parse::just_parse (state, input_stream));
        xml::parse::post_processing_passes (function, &state);
      }
    , boost::format 
        ( "ERROR: connect-eureka to non-existent output port %1%"
          " in transition %2% at %3%"
        )
    % port_name
    % trans_name
    % "[<stdin>:11:5]"
    );
}
