// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/ostream/callback/print.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/test_case.hpp>
#include <util-generic/write_file.hpp>

#include <FMT/boost/filesystem/path.hpp>
#include <fmt/core.h>
#include <numeric>

namespace
{
  template<typename T> std::string as_written (T const& content)
  {
    return fhg::util::ostream::callback::print<T>
      (fhg::util::ostream::callback::id<T>(), content).string();
  }
}

FHG_UTIL_TESTING_TEMPLATED_CASE (write_file_stores_content, int, std::string)
{
  fhg::util::temporary_path const path;
  fhg::util::temporary_file const file
    (path / ::boost::filesystem::unique_path());

  auto const content (fhg::util::testing::random<T>{}());

  fhg::util::write_file (file, content);

  BOOST_REQUIRE_EQUAL (fhg::util::read_file (file), as_written (content));
}

BOOST_AUTO_TEST_CASE (write_file_overwrites)
{
  fhg::util::temporary_path const path;
  fhg::util::temporary_file const file
    (path / ::boost::filesystem::unique_path());

  std::string const base {fhg::util::testing::random<std::string>()()};

  fhg::util::write_file (file, base + "1");
  fhg::util::write_file (file, base + "2");

  BOOST_REQUIRE_EQUAL (fhg::util::read_file (file), base + "2");
}

BOOST_AUTO_TEST_CASE (write_file_throws_when_file_can_not_opened)
{
  ::boost::filesystem::path const file ("/");

  fhg::util::testing::require_exception
    ( [&file]()
      {
        fhg::util::write_file (file, fhg::util::testing::random<int>()());
      }
    , std::runtime_error {fmt::format ("Could not open {} for writing.", file)}
    );
}

namespace
{
  class throwing_ostream_modifier : public fhg::util::ostream::modifier
  {
  public:
    throwing_ostream_modifier (std::string const& message)
      : _message (message)
    {}
    virtual std::ostream& operator() (std::ostream&) const override
    {
      throw std::logic_error (_message);
    }

  private:
    std::string const _message;
  };

  std::string random_cstr_comparable()
  {
    return fhg::util::testing::random<std::string>{}
      (fhg::util::testing::random<char>::any_without_zero());
  }
}

BOOST_AUTO_TEST_CASE (write_file_throws_on_output_failure)
{
  fhg::util::temporary_path const path;
  fhg::util::temporary_file const file
    (path / ::boost::filesystem::unique_path());

  std::string const message {random_cstr_comparable()};

  fhg::util::testing::require_exception
    ( [&file, &message]()
      {
        fhg::util::write_file (file, throwing_ostream_modifier (message));
      }
    , std::logic_error (message)
    );

  BOOST_REQUIRE (::boost::filesystem::exists (file));
}

namespace
{
  class file_removing_ostream_modifier : public fhg::util::ostream::modifier
  {
  public:
    file_removing_ostream_modifier (::boost::filesystem::path const& file)
      : _file (file)
    {}
    virtual std::ostream& operator() (std::ostream& os) const override
    {
      ::boost::filesystem::remove (_file);

      return os << fhg::util::testing::random<std::string>()();
    }

  private:
    ::boost::filesystem::path const _file;
  };
}

BOOST_AUTO_TEST_CASE (write_file_has_file_open_when_doing_output)
{
  fhg::util::temporary_path const path;
  fhg::util::temporary_file const _
    (path / ::boost::filesystem::unique_path());
  ::boost::filesystem::path const file (_);

  fhg::util::write_file (file, file_removing_ostream_modifier (file));

  BOOST_REQUIRE (!::boost::filesystem::exists (file));
}

//! \todo how to test write errors? disk full?
