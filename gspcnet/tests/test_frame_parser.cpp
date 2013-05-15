#define BOOST_TEST_MODULE GspcNetFrameParser
#include <boost/test/unit_test.hpp>

#include <cstring>
#include <iostream>
#include <algorithm>

#include <gspc/net.hpp>
#include <gspc/net/parse/parser.hpp>
#include <gspc/net/frame_io.hpp>

BOOST_AUTO_TEST_CASE (invalid_frame_start)
{
  // only some examples
  const char to_check[] =
    { 0 // NUL
    , 1 // SOH Ctrl+A
    , 2 // STX Ctrl+B
    , 3 // ETX Ctrl+C
    , 4 // EOT Ctrl+D
    , 5 // ENQ Ctrl+E
    , 6 // ACK Ctrl+F
    , 7 // BEL Ctrl+G
    , 8 // BS  Ctrl+H
    , 9 // HT  Ctrl+I

    // , 10 // LF Ctrl+J is allowed

    , 11 // VT  Ctrl+K
    , 12 // FF  Ctrl+L

    // , 13 // CR  Ctrl+M is allowed

    , 127 // DEL
  };

  for (size_t i = 0 ; i < sizeof (to_check) ; ++i)
  {
    gspc::net::frame frame;
    gspc::net::parse::parser parser;
    gspc::net::parse::result_t result;

    const char input [] = {to_check [i]};

    result = parser.parse ( input
                          , input + sizeof (input)
                          , frame
                          );

    BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FAILED);
    BOOST_REQUIRE_EQUAL (result.consumed, sizeof(input));
  }
}

BOOST_AUTO_TEST_CASE (invalid_empty_frame)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char *input = "\r\r";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input) + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FAILED);
  BOOST_REQUIRE_EQUAL (result.consumed, strlen (input));
}

BOOST_AUTO_TEST_CASE (test_invalid_command)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char *input = "FOO\r\r";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input) + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FAILED);
  BOOST_REQUIRE_EQUAL (result.consumed, strlen (input));
}

BOOST_AUTO_TEST_CASE (test_heartbeat_frame)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char *input = "\n";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input) + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FINISHED);
  // heartbeats don't want a NULL byte
  BOOST_REQUIRE_EQUAL (result.consumed, strlen (input));

  BOOST_CHECK_EQUAL (frame.get_command (), "");
  BOOST_CHECK_EQUAL (frame.get_header ().size (), 0);
  BOOST_CHECK_EQUAL (frame.get_body_as_string (), "");
}

BOOST_AUTO_TEST_CASE (test_heartbeat_crlf_frame)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char *input = "\r\n";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input) + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FINISHED);
  // heartbeats don't want a NULL byte
  BOOST_REQUIRE_EQUAL (result.consumed, strlen (input));

  BOOST_CHECK_EQUAL (frame.get_command (), "");
  BOOST_CHECK_EQUAL (frame.get_header ().size (), 0);
}

BOOST_AUTO_TEST_CASE (test_empty_header_empty_body)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char *input = "CONNECT\n\n";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input) + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FINISHED);
  BOOST_REQUIRE_EQUAL (result.consumed, strlen (input) + 1);

  BOOST_CHECK_EQUAL (frame.get_command (), "CONNECT");
  BOOST_CHECK_EQUAL (frame.get_header ().size (), 0);
}

BOOST_AUTO_TEST_CASE (test_command_need_more_data)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char *input = "CONNECT";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input)
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_NEED_MORE_DATA);
  BOOST_REQUIRE_EQUAL (result.consumed, strlen (input));
}

BOOST_AUTO_TEST_CASE (test_empty_header_empty_body_crlf)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char *input = "CONNECT\r\n\r\n";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input) + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FINISHED);
  BOOST_REQUIRE_EQUAL (result.consumed, strlen (input) + 1);

  BOOST_CHECK_EQUAL (frame.get_command (), "CONNECT");
  BOOST_CHECK_EQUAL (frame.get_header ().size (), 0);
  BOOST_CHECK_EQUAL (frame.get_body_as_string (), "");
}

BOOST_AUTO_TEST_CASE (test_header)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char *input = "CONNECT\nfoo:bar\nbaz:bam\n\n";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input) + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FINISHED);
  BOOST_REQUIRE_EQUAL (result.consumed, strlen (input) + 1);

  BOOST_CHECK_EQUAL (frame.get_command (), "CONNECT");
  BOOST_CHECK_EQUAL (frame.get_header ().size (), 2);
  BOOST_REQUIRE (frame.has_header ("foo"));
  BOOST_CHECK_EQUAL (*frame.get_header ("foo"), "bar");
  BOOST_REQUIRE (frame.has_header ("baz"));
  BOOST_CHECK_EQUAL (*frame.get_header ("baz"), "bam");
  BOOST_CHECK_EQUAL (frame.get_body_as_string (), "");
}

BOOST_AUTO_TEST_CASE (test_invalid_header)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char *input = "CONNECT\n:bar\n\n";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input) + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FAILED);
  BOOST_REQUIRE (result.consumed > 0);
  BOOST_REQUIRE (result.consumed < strlen (input));
}

BOOST_AUTO_TEST_CASE (test_header_empty_value)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char input[] = "CONNECT\nfoo:\n\n";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + sizeof(input)
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FINISHED);
  BOOST_REQUIRE (result.consumed > 0);
  BOOST_REQUIRE (result.consumed == sizeof(input));
  BOOST_REQUIRE (frame.has_header ("foo"));
  BOOST_REQUIRE_EQUAL (*frame.get_header ("foo"), "");
}

BOOST_AUTO_TEST_CASE (test_empty_body_with_content_length)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char input[] = "SEND\ncontent-length:0\n\n";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + sizeof(input)
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FINISHED);
  BOOST_REQUIRE (result.consumed == sizeof(input));
  BOOST_REQUIRE (frame.has_header ("content-length"));
  BOOST_REQUIRE_EQUAL (*frame.get_header ("content-length"), "0");
}

BOOST_AUTO_TEST_CASE (test_header_value_with_spaces)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char input[] = "CONNECT\nfoo: 1  2   3    4 \n\n";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + sizeof(input)
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FINISHED);
  BOOST_REQUIRE (result.consumed > 0);
  BOOST_REQUIRE (result.consumed == sizeof(input));
  BOOST_REQUIRE (frame.has_header ("foo"));
  BOOST_REQUIRE_EQUAL (*frame.get_header ("foo"), " 1  2   3    4 ");
}

BOOST_AUTO_TEST_CASE (test_content_length)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char input[] = "CONNECT\ncontent-length:5\n\n12345\x00\x01\x02\x03\x04";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input) + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FINISHED);
  BOOST_REQUIRE (result.consumed > 0);
  BOOST_REQUIRE (result.consumed == sizeof (input) - 5);

  BOOST_REQUIRE_EQUAL (frame.get_body_as_string (), "12345");
}

BOOST_AUTO_TEST_CASE (test_content_length_body_too_long)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char input[] = "CONNECT\ncontent-length:5\n\n123456\x00";
  gspc::net::frame frame;

  result = parser.parse ( input
                        , input + strlen (input) + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FAILED);
  BOOST_REQUIRE (result.consumed > 0);
  BOOST_REQUIRE_EQUAL (result.consumed, sizeof (input) - 2);
}

BOOST_AUTO_TEST_CASE (test_binary_body)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char bytes[] = "A\ncontent-length:5\n\n\x00\x01\x02\x03\x04";

  gspc::net::frame frame;

  result = parser.parse ( bytes
                        , bytes + sizeof (bytes)
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_FINISHED);

  // the trailing 0 is not consumed
  BOOST_CHECK (result.consumed == sizeof (bytes));
  BOOST_CHECK (result.consumed == 26);

  BOOST_REQUIRE_EQUAL (bytes [result.consumed-1], '\x00');

  BOOST_REQUIRE_EQUAL (frame.get_body ().size (), 5);
  BOOST_REQUIRE_EQUAL (frame.get_body ()[0], '\x00');
  BOOST_REQUIRE_EQUAL (frame.get_body ()[1], '\x01');
  BOOST_REQUIRE_EQUAL (frame.get_body ()[2], '\x02');
  BOOST_REQUIRE_EQUAL (frame.get_body ()[3], '\x03');
  BOOST_REQUIRE_EQUAL (frame.get_body ()[4], '\x04');
}

BOOST_AUTO_TEST_CASE (test_unfinished)
{
  gspc::net::parse::parser parser;
  gspc::net::parse::result_t result;

  const char bytes[] = "A";

  gspc::net::frame frame;

  result = parser.parse ( bytes
                        , bytes + 1
                        , frame
                        );

  BOOST_REQUIRE_EQUAL (result.state, gspc::net::parse::PARSE_NEED_MORE_DATA);
  BOOST_REQUIRE_EQUAL (result.consumed, 1);
  BOOST_REQUIRE_EQUAL (bytes[result.consumed-1], 'A');
}

BOOST_AUTO_TEST_CASE (test_nul_in_sstream)
{
  std::stringstream sstr;
  sstr << '\0';
  sstr << '\1';
  sstr << '\2';
  sstr << '\3';

  sstr << '\0';
  sstr << '\1';
  sstr << '\2';
  sstr << '\3';

  const std::string result (sstr.str ());

  BOOST_REQUIRE_EQUAL (result.size (), 8);
  BOOST_REQUIRE_EQUAL (result [0], '\0');
  BOOST_REQUIRE_EQUAL (result [1], '\1');
  BOOST_REQUIRE_EQUAL (result [2], '\2');
  BOOST_REQUIRE_EQUAL (result [3], '\3');
  BOOST_REQUIRE_EQUAL (result [4], '\0');
  BOOST_REQUIRE_EQUAL (result [5], '\1');
  BOOST_REQUIRE_EQUAL (result [6], '\2');
  BOOST_REQUIRE_EQUAL (result [7], '\3');
}
