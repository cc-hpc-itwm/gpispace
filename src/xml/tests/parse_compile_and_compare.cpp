// bernd.loerwald@itwm.fraunhofer.de

#include <parser_fixture.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_FIXTURE_TEST_CASE (simple_pipe_elimination, parser_fixture)
{
  parse ("simple_pipe_elimination_token.xpnet");
  post_processing_passes();
  require_same (xml_to_we(), "simple_pipe_elimination_token.pnet");
}
