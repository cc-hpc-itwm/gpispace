// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE network: measure and assert roundtrip \
                          time with multiple binaries

#include <fhg/syscall.hpp>
#include <fhg/util/boost/test/printer/chrono.hpp>
#include <fhg/util/getenv.hpp>
#include <fhg/util/hostname.hpp>
#include <fhg/util/macros.hpp>
#include <fhg/util/split.hpp>

#include <share/process/process.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

#include <array>
#include <chrono>

namespace
{
  template<typename Duration>
    void do_test ( std::string client_host
                 , std::size_t payload_size
                 , Duration IFDEF_NDEBUG (allowed_roundtrip_time)
                 )
  {
    BOOST_REQUIRE_EQUAL
      (boost::unit_test::framework::master_test_suite().argc, 2);

    boost::filesystem::path const multiple_binaries_binary
      (boost::unit_test::framework::master_test_suite().argv[1]);

    //! \note two ints, two newlines, padding
    std::array<char, 10 + 1 + 10 + 1 + 1> server_output_buffer;
    server_output_buffer.fill (0);
    process::execute_return_type const server_return_values
      ( process::execute
          ( multiple_binaries_binary.string()
          , {nullptr, 0}
          , {server_output_buffer.data(), server_output_buffer.size()}
          , {}
          , {}
          )
      );
    //! \note losing server when test fails between now and
    //! construction of at_scope_exit_kill_server.

    BOOST_REQUIRE_EQUAL (server_return_values.exit_code, 0);
    BOOST_REQUIRE_LT
      (server_return_values.bytes_read_stdout, server_output_buffer.size());

    std::list<std::string> const lines
      ( fhg::util::split<std::string, std::string>
         ({server_output_buffer.cbegin(), server_output_buffer.cend()}, '\n')
      );

    unsigned int const port
      (boost::lexical_cast<unsigned int> (*lines.begin()));
    unsigned int const pid
      (boost::lexical_cast<unsigned int> (*(++lines.begin())));

    struct at_scope_exit_kill_server
    {
      unsigned int const _pid;
      ~at_scope_exit_kill_server()
      {
        try
        {
          fhg::syscall::kill (_pid, SIGKILL);
        }
        catch (boost::system::system_error const& err)
        {
          if (err.code() == boost::system::errc::no_such_process)
          {
            //! \note ignore: it properly terminated
            return;
          }
          throw;
        }
      }
    } const _ {pid};

    //! \note one long, "µs", one newline, padding
    std::array<char, 20 + 2 + 1 + 1 + 2000> client_output_buffer;
    client_output_buffer.fill (0);
    process::execute_return_type const client_return_values
      ( process::execute
          ( ( boost::format ("/usr/bin/ssh %1% /usr/bin/env LD_LIBRARY_PATH=%6% %2% %3% %4% %5%")
            % client_host
            % boost::filesystem::canonical (multiple_binaries_binary).string()
            % fhg::util::hostname()
            % port
            % payload_size
            % fhg::util::getenv ("LD_LIBRARY_PATH").get_value_or ("")
            ).str()
          , {nullptr, 0}
          , {client_output_buffer.data(), client_output_buffer.size()}
          , {}
          , {}
          )
      );

    BOOST_REQUIRE_EQUAL (client_return_values.exit_code, 0);
    BOOST_REQUIRE_LT
      (client_return_values.bytes_read_stdout, client_output_buffer.size());

    std::chrono::microseconds const roundtrip_time
      (std::strtoull (client_output_buffer.data(), nullptr, 10));

#ifdef NDEBUG
    BOOST_REQUIRE_LE (roundtrip_time, allowed_roundtrip_time);
#endif
  }
}

BOOST_AUTO_TEST_CASE (on_same_host_1kb)
{
  do_test (fhg::util::hostname(), 1UL * (1 << 10), std::chrono::microseconds (100));
}

BOOST_AUTO_TEST_CASE (on_same_host_1mb)
{
  do_test (fhg::util::hostname(), 1UL * (1 << 20), std::chrono::milliseconds (10));
}
