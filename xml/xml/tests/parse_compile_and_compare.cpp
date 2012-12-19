// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE parse_compile_and_compare

#include <xml/tests/parser_fixture.hpp>

BOOST_FIXTURE_TEST_CASE (simple_pipe_elimination, parser_fixture)
{
  parse ("simple_pipe_elimination_token.xpnet");
  post_processing_passes();
  ::we::mgmt::type::activity_t activity ();

  require_same (xml_to_we(), "simple_pipe_elimination_token.pnet");
}
