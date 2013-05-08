#define BOOST_TEST_MODULE ProcessBasics
#include <boost/test/unit_test.hpp>

#include <fhglog/minimal.hpp>
#include <csignal>

#include <process.hpp>

struct Setup
{
  Setup ()
  {
    FHGLOG_SETUP();
  }
};

BOOST_GLOBAL_FIXTURE( Setup )

struct F
{
  F ()
  {
  }

  process::file_const_buffer_list m_input_files;
  process::file_buffer_list       m_output_files;

  enum { max_buffer_length = 1024 };

  char m_input [max_buffer_length];
  char m_output [max_buffer_length];
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE (no_such_file_or_directory)
{
  process::execute_return_type result;

  result =
    process::execute ( "no.such.file.373f5565-a8f4-42de-a515-9271812fedd6"
                     , process::const_buffer (m_input, max_buffer_length)
                     , process::buffer (m_output, max_buffer_length)
                     , m_input_files
                     , m_output_files
                     );

  BOOST_REQUIRE_EQUAL (result.exit_code, 127);
}

BOOST_AUTO_TEST_CASE (not_executable)
{
  process::execute_return_type result;

  result =
    process::execute ( "/etc/passwd"
                     , process::const_buffer (m_input, max_buffer_length)
                     , process::buffer (m_output, max_buffer_length)
                     , m_input_files
                     , m_output_files
                     );
  BOOST_REQUIRE_EQUAL (result.exit_code, 126);
}

BOOST_AUTO_TEST_SUITE_END()
