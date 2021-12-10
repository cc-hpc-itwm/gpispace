// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <rif/started_process_promise.hpp>

#include <util-generic/executable_path.hpp>
#include <util-generic/serialization/exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/warning.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/format.hpp>
#include <boost/mpl/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

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
      return fhg::util::suppress_warning::shorten_64_to_32_with_check<int>
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
        ( fhg::util::syscall::read
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

  fhg::util::testing::require_exception
    ( [&arguments]
      {
        int argc (arguments.argc());
        char** argv (arguments.argv());
        fhg::rif::started_process_promise promise (argc, argv);
      }
    , std::invalid_argument
        (( ::boost::format ("Usage: %1% <pipefd> [args]...")
         % fhg::util::executable_path()
         ).str()
        )
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
      fhg::rif::started_process_promise promise (argc, argv);

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
      (fhg::rif::started_process_promise::end_sentinel_value());

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
  fhg::util::syscall::pipe (pipefd, O_NONBLOCK);

  argument_builder arguments
    {"./application.exe", std::to_string (pipefd[1]).c_str()};

  int argc (arguments.argc());
  char** argv (arguments.argv());

  {
    fhg::rif::started_process_promise promise (argc, argv);
    promise.set_result();

    check_something_with_sentinel_was_written (pipefd[0]);
  }
}

BOOST_AUTO_TEST_CASE (setting_exception_writes_something_ending_with_sentinel_value_to_given_fd)
{
  int pipefd[2];
  fhg::util::syscall::pipe (pipefd, O_NONBLOCK);

  argument_builder arguments
    {"./application.exe", std::to_string (pipefd[1]).c_str()};

  int argc (arguments.argc());
  char** argv (arguments.argv());

  {
    fhg::rif::started_process_promise promise (argc, argv);
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
           , fhg::util::testing::randoms<std::vector<std::string>> (100)
           };
  }
}

BOOST_DATA_TEST_CASE
  (result_serializes_a_vector_of_messages, messagess(), messages)
{
  int pipefd[2];
  fhg::util::syscall::pipe (pipefd, O_NONBLOCK);

  argument_builder arguments
    {"./application.exe", std::to_string (pipefd[1]).c_str()};

  int argc (arguments.argc());
  char** argv (arguments.argv());

  {
    fhg::rif::started_process_promise promise (argc, argv);
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
  auto const exception_arg (fhg::util::testing::random<std::string>{}());

  int pipefd[2];
  fhg::util::syscall::pipe (pipefd, O_NONBLOCK);

  argument_builder arguments
    {"./application.exe", std::to_string (pipefd[1]).c_str()};

  int argc (arguments.argc());
  char** argv (arguments.argv());

  {
    fhg::rif::started_process_promise promise (argc, argv);
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

    fhg::util::testing::require_exception
      ( [&serialized_exception]
        {
          std::rethrow_exception
            ( fhg::util::serialization::exception::deserialize
                (serialized_exception)
            );
        }
      , Exception (exception_arg)
      );
  }
}
