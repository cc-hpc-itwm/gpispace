// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE "invoking process::execute in parallel is safe"
#include <boost/test/unit_test.hpp>

#include <process.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/test/printer/vector.hpp>

#include <boost/thread/scoped_thread.hpp>

namespace
{
  struct thread_data_type
  {
    thread_data_type (const std::size_t thread_id)
      : stdin (std::to_string (thread_id) + " STDIN")
      , expected_stdout (std::to_string (thread_id) + " STDOUT")
      , expected_stderr (std::to_string (thread_id) + " STDERR")
      , file_in (std::to_string (thread_id) + " FILE_IN")
      , expected_file_out (std::to_string (thread_id) + " FILE_OUT")
      , result (0)
    {}

    void verify()
    {
      BOOST_REQUIRE_EQUAL (result.exit_code, 0);
      BOOST_REQUIRE_EQUAL (result.bytes_written_stdin, stdin.size());
      BOOST_REQUIRE_EQUAL (result.bytes_read_stdout, expected_stdout.size());
      BOOST_REQUIRE_EQUAL (result.bytes_read_stderr, expected_stderr.size());

      //! \todo also do for written_files_input if information available
      const std::vector<std::size_t> expected_bytes_read_files_output
        {expected_file_out.size()};
      BOOST_REQUIRE_EQUAL
        (result.bytes_read_files_output, expected_bytes_read_files_output);

      BOOST_REQUIRE_EQUAL (stdout, expected_stdout);
      BOOST_REQUIRE_EQUAL (stderr, expected_stderr);
      BOOST_REQUIRE_EQUAL (file_out, expected_file_out);
    }

    const std::string stdin;
    const std::string expected_stdout;
    const std::string expected_stderr;
    const std::string file_in;
    const std::string expected_file_out;

    process::execute_return_type result;

    std::string stdout;
    std::string stderr;
    std::string file_out;
  };

  void thread
    (const std::size_t thread_id, std::string command, thread_data_type& data)
  {
    constexpr std::size_t buffer_size (1024);
    std::array<char, buffer_size> buffer_stdout;
    std::vector<char> buffer_stderr;
    std::array<char, buffer_size> buffer_file_out;

    data.result =
      ( process::execute
        ( command + " " + std::to_string (thread_id) + " %FILE_IN% %FILE_OUT%"
        , {data.stdin.c_str(), data.stdin.size()}
        , {buffer_stdout.data(), buffer_stdout.size()}
        , buffer_stderr
        , {{data.file_in.c_str(), data.file_in.size(), "%FILE_IN%"}}
        , {{buffer_file_out.data(), buffer_file_out.size(), "%FILE_OUT%"}}
        )
      );

    data.stdout = std::string
      ( buffer_stdout.begin()
      , buffer_stdout.begin() + data.result.bytes_read_stdout
      );
    data.stderr = std::string
      ( buffer_stderr.begin()
      , buffer_stderr.begin() + data.result.bytes_read_stderr
      );
    data.file_out = std::string
      ( buffer_file_out.begin()
      , buffer_file_out.begin() + data.result.bytes_read_files_output[0]
      );
  }

  void run_n_and_verify (const std::size_t count)
  {
    BOOST_REQUIRE_GT (boost::unit_test::framework::master_test_suite().argc, 1);
    const std::string command
      (boost::unit_test::framework::master_test_suite().argv[1]);

    std::list<thread_data_type> thread_data;
    {
      std::list<boost::strict_scoped_thread<boost::join_if_joinable>> threads;
      for (std::size_t thread_id (0); thread_id < count; ++thread_id)
      {
        thread_data.emplace_back (thread_id);
        threads.emplace_back
          (&thread, thread_id, command, std::ref (thread_data.back()));
      }
    }
    for (thread_data_type& data : thread_data)
    {
      data.verify();
    }
  }
}

BOOST_AUTO_TEST_CASE (one_thread)
{
  run_n_and_verify (1);
}

BOOST_AUTO_TEST_CASE (ten_threads)
{
  run_n_and_verify (10);
}

BOOST_AUTO_TEST_CASE (hundred_threads)
{
  run_n_and_verify (100);
}
