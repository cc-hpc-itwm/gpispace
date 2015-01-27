// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE "child or parent stops consuming while there is more"
#include <boost/test/unit_test.hpp>

#include <process.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>

#include <boost/optional.hpp>

#include <signal.h>

namespace
{
  void consume ( int exit_code
               , std::size_t input_buffer_size
               , std::size_t output_buffer_size
               , std::size_t to_read
               , boost::optional<std::size_t> written
               , std::size_t read
               )
  {
    char m_input [input_buffer_size];
    char m_output [output_buffer_size];

    process::execute_return_type result
      ( process::execute ( "/usr/bin/head -c " + std::to_string (to_read)
                         , process::const_buffer (m_input, sizeof (m_input))
                         , process::buffer (m_output, sizeof (m_output))
                         , {}
                         , {}
                         )
      );

    BOOST_REQUIRE_EQUAL (result.exit_code, exit_code);
    if (written)
    {
      BOOST_REQUIRE_EQUAL (result.bytes_written_stdin, *written);
    }
    BOOST_REQUIRE_EQUAL (result.bytes_read_stdout, read);
    BOOST_REQUIRE_EQUAL (result.bytes_read_stderr, 0);
  }
}

BOOST_AUTO_TEST_CASE (consume_everything)
{
  std::size_t const size (1 << 20);
  consume (0, size, size, size, size, size);
}

BOOST_AUTO_TEST_CASE (consume_half)
{
  std::size_t const size (1 << 20);
  consume (0, size, size / 2,  size / 2, size / 2, size / 2);
}

BOOST_AUTO_TEST_CASE (consume_nothing)
{
  std::size_t const size (1 << 20);
  consume (0, size, size, 0UL, 0UL, 0UL);
}

BOOST_AUTO_TEST_CASE (try_consume_more_than_available)
{
  std::size_t const size (1 << 20);
  consume (0, size, size * 2, size * 2, size, size);
}

BOOST_AUTO_TEST_CASE (try_produce_more_than_available)
{
  std::size_t const size (1 << 20);
  //! \note Not possible to assert a specific amount written, as child
  //! may consume more from stdin before getting SIGPIPE for stdout
  consume (128 + SIGPIPE, size * 2, size, size * 2, boost::none, size);
}
