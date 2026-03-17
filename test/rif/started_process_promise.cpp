// Copyright (C) 2015-2016,2019-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/rif/started_process_promise.hpp>

#include <gspc/util/executable_path.hpp>
#include <gspc/util/serialization/exception.hpp>
#include <gspc/util/syscall.hpp>
#include <gspc/testing/printer/vector.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/util/warning.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/mpl/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/test/data/test_case.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>
#include <string>
#include <vector>

#include <fcntl.h>

namespace
{
  struct argument_builder
  {
    argument_builder (std::initializer_list<std::string> args)
    {
      std::vector<std::size_t> offsets;
      for (std::string const& arg : args)
      {
        offsets.emplace_back (_raw.size());
        _raw.insert (_raw.end(), arg.begin(), arg.end());
        _raw.emplace_back ('\0');
      }
      for (std::size_t const& offset : offsets)
      {
        _pointers.emplace_back (_raw.data() + offset);
      }
    }

    int argc()
    {
      return gspc::util::suppress_warning::shorten_64_to_32_with_check<int>
        (_pointers.size(), "c++ standard defines argc as int");
    }
    char** argv()
    {
      return _pointers.data();
    }
    char* at (std::size_t index)
    {
      return _pointers.at (index);
    }

  private:
    std::vector<char> _raw;
    std::vector<char*> _pointers;
  };

  std::vector<char> read_from_fd (int fd)
  {
    std::vector<char> buffer (1024);
    std::size_t total_read (0);
    while (true)
    {
      std::size_t const to_read (buffer.size() - total_read);
      std::size_t const read
        ( gspc::util::syscall::read
            (fd, buffer.data() + total_read, to_read)
        );

      total_read += read;

      if (read == 0)
      {
        break;
      }

      buffer.resize (buffer.size() * 2);
    }
    buffer.resize (total_read);
    return buffer;
  }
}

BOOST_AUTO_TEST_CASE (missing_special_argument_throws)
{
  argument_builder arguments
    {"./application.exe"};

  gspc::testing::require_exception
    ( [&arguments]
      {
        int argc (arguments.argc());
        char** argv (arguments.argv());
        gspc::rif::started_process_promise promise (argc, argv);
      }
    , std::invalid_argument
        { fmt::format ( "Usage: {} <pipefd> [args]..."
                      , gspc::util::executable_path()
                      )
        }
    );

}

BOOST_AUTO_TEST_CASE (only_special_argument_is_removed_and_restored)
{
  argument_builder arguments
    {"./application.exe", "401", "--actual-argument"};

  {
    int argc (arguments.argc());
    char** argv (arguments.argv());
    std::vector<char*> const expected {arguments.at (0), arguments.at (2)};

    {
      gspc::rif::started_process_promise promise (argc, argv);

      BOOST_REQUIRE_EQUAL (argc, expected.size());
      BOOST_REQUIRE_EQUAL_COLLECTIONS
        (argv, argv + argc, expected.begin(), expected.end());
    }

    BOOST_REQUIRE_EQUAL (argc, arguments.argc());
    BOOST_REQUIRE_EQUAL_COLLECTIONS
      ( argv, argv + argc
      , arguments.argv(), arguments.argv() + arguments.argc()
      );
  }
}

namespace
{
  void check_something_with_sentinel_was_written (int fd)
  {
    std::vector<char> const buffer (read_from_fd (fd));

    std::string const sentinel
      (gspc::rif::started_process_promise::end_sentinel_value());

    BOOST_REQUIRE_GT (buffer.size(), sentinel.size());
    BOOST_REQUIRE_EQUAL
      ( std::string ( buffer.data() + buffer.size() - sentinel.size()
                    , sentinel.size()
                    )
      , sentinel
      );
  }
}

BOOST_AUTO_TEST_CASE (setting_result_writes_something_ending_with_sentinel_value_to_given_fd)
{
  int pipefd[2];
  gspc::util::syscall::pipe (pipefd, O_NONBLOCK);

  argument_builder arguments
    {"./application.exe", std::to_string (pipefd[1])};

  int argc (arguments.argc());
  char** argv (arguments.argv());

  {
    gspc::rif::started_process_promise promise (argc, argv);
    promise.set_result();

    check_something_with_sentinel_was_written (pipefd[0]);
  }
}

BOOST_AUTO_TEST_CASE (setting_exception_writes_something_ending_with_sentinel_value_to_given_fd)
{
  int pipefd[2];
  gspc::util::syscall::pipe (pipefd, O_NONBLOCK);

  argument_builder arguments
    {"./application.exe", std::to_string (pipefd[1])};

  int argc (arguments.argc());
  char** argv (arguments.argv());

  {
    gspc::rif::started_process_promise promise (argc, argv);
    promise.set_exception
      (std::make_exception_ptr (std::runtime_error ("foo")));

    check_something_with_sentinel_was_written (pipefd[0]);
  }
}

namespace
{
  std::vector<std::vector<std::string>> messagess()
  {
    return { {}
           , {"foo"}
           , gspc::testing::randoms<std::vector<std::string>> (100)
           };
  }
}

BOOST_DATA_TEST_CASE
  (result_serializes_a_vector_of_messages, messagess(), messages)
{
  int pipefd[2];
  gspc::util::syscall::pipe (pipefd, O_NONBLOCK);

  argument_builder arguments
    {"./application.exe", std::to_string (pipefd[1])};

  int argc (arguments.argc());
  char** argv (arguments.argv());

  {
    gspc::rif::started_process_promise promise (argc, argv);
    promise.set_result (messages);

    std::vector<char> const data (read_from_fd (pipefd[0]));
    std::string str (data.begin(), data.end());
    std::istringstream sstr (str);
    ::boost::archive::text_iarchive archive (sstr);

    bool result;
    archive & result;

    BOOST_REQUIRE_EQUAL (result, true);

    std::vector<std::string> deserialized_messages;
    archive & deserialized_messages;

    BOOST_REQUIRE_EQUAL (deserialized_messages, messages);
  }
}

using exception_types = ::boost::mpl::list<std::runtime_error, std::range_error>;

BOOST_AUTO_TEST_CASE_TEMPLATE
  (exceptions_are_serialized_using_util_generic, Exception, exception_types)
{
  auto const exception_arg (gspc::testing::random<std::string>{}());

  int pipefd[2];
  gspc::util::syscall::pipe (pipefd, O_NONBLOCK);

  argument_builder arguments
    {"./application.exe", std::to_string (pipefd[1])};

  int argc (arguments.argc());
  char** argv (arguments.argv());

  {
    gspc::rif::started_process_promise promise (argc, argv);
    promise.set_exception (std::make_exception_ptr (Exception (exception_arg)));

    std::vector<char> const data (read_from_fd (pipefd[0]));
    std::string str (data.begin(), data.end());
    std::istringstream sstr (str);
    ::boost::archive::text_iarchive archive (sstr);

    bool result;
    archive & result;

    BOOST_REQUIRE_EQUAL (result, false);

    std::string serialized_exception;
    archive & serialized_exception;

    gspc::testing::require_exception
      ( [&serialized_exception]
        {
          std::rethrow_exception
            ( gspc::util::serialization::exception::deserialize
                (serialized_exception)
            );
        }
      , Exception (exception_arg)
      );
  }
}
