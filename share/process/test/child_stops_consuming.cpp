// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE "child stops consuming input while there is more"
#include <boost/test/unit_test.hpp>

#include <process.hpp>

BOOST_AUTO_TEST_CASE (consume_everything)
{
  char m_input [2 << 20];
  char m_output [sizeof (m_input)];

  const std::size_t to_read (sizeof (m_input));

  process::execute_return_type result
    ( process::execute ( "/usr/bin/head -c " + std::to_string (to_read)
                       , process::const_buffer (m_input, sizeof (m_input))
                       , process::buffer (m_output, sizeof (m_output))
                       , {}
                       , {}
                       )
    );

  BOOST_REQUIRE_EQUAL (result.exit_code, 0);
  BOOST_REQUIRE_EQUAL (result.bytes_written_stdin, to_read);
  BOOST_REQUIRE_EQUAL (result.bytes_read_stdout, to_read);
  BOOST_REQUIRE_EQUAL (result.bytes_read_stderr, 0);
}

BOOST_AUTO_TEST_CASE (consume_half)
{
  char m_input [2 << 20];
  char m_output [sizeof (m_input)];

  const std::size_t to_read (sizeof (m_input) / 2);

  process::execute_return_type result
    ( process::execute ( "/usr/bin/head -c " + std::to_string (to_read)
                       , process::const_buffer (m_input, sizeof (m_input))
                       , process::buffer (m_output, sizeof (m_output))
                       , {}
                       , {}
                       )
    );

  BOOST_REQUIRE_EQUAL (result.exit_code, 0);
  BOOST_REQUIRE_EQUAL (result.bytes_written_stdin, to_read);
  BOOST_REQUIRE_EQUAL (result.bytes_read_stdout, to_read);
  BOOST_REQUIRE_EQUAL (result.bytes_read_stderr, 0);
}

BOOST_AUTO_TEST_CASE (consume_nothing)
{
  char m_input [2 << 20];
  char m_output [sizeof (m_input)];

  const std::size_t to_read (0);

  process::execute_return_type result
    ( process::execute ( "/usr/bin/head -c " + std::to_string (to_read)
                       , process::const_buffer (m_input, sizeof (m_input))
                       , process::buffer (m_output, sizeof (m_output))
                       , {}
                       , {}
                       )
    );

  BOOST_REQUIRE_EQUAL (result.exit_code, 0);
  BOOST_REQUIRE_EQUAL (result.bytes_written_stdin, to_read);
  BOOST_REQUIRE_EQUAL (result.bytes_read_stdout, to_read);
  BOOST_REQUIRE_EQUAL (result.bytes_read_stderr, 0);
}
