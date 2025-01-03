// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

//! \note: that test might fail _to output_ differences as the output
//! stream might be redirected :(

#include <boost/test/unit_test.hpp>

#include <util-generic/ostream/redirect.hpp>
#include <util-generic/testing/random.hpp>

#include <fmt/core.h>
#include <iostream>
#include <sstream>
#include <vector>

namespace
{
  std::string random_single_line_string()
  {
    return fhg::util::testing::random<std::string>{}
      (fhg::util::testing::random<std::string>::except ("\n"));
  }
}

BOOST_AUTO_TEST_CASE (redirect_cout_to_vector)
{
  std::vector<std::string> lines;

  fhg::util::ostream::redirect const redirecter
    ( std::cout
    , [&lines] (std::string const& line)
      {
        lines.emplace_back (line);
      }
    );

  std::vector<std::string> expected;

  for (int i (0); i < 10; ++i)
  {
    auto const line (random_single_line_string());

    std::cout << line << '\n';

    expected.emplace_back (line);

    BOOST_REQUIRE_EQUAL (lines.size(), i + 1);
    BOOST_REQUIRE_EQUAL (lines.back(), expected.back());
  }

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (lines.begin(), lines.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE (prepend_lines_in_scope)
{
  auto prepend
    ( [] (unsigned k, std::string const& s)
      {
        return fmt::format ("{}: {}", k, s);
      }
    );
  std::ostringstream oss;
  unsigned n (0);

  std::string expected;

  {
    fhg::util::ostream::redirect const redirecter
      ( oss
      , [&oss, &n, &prepend] (std::string const& line)
        {
          oss << prepend (n++, line);
        }
      );

    for (unsigned int i (0); i < 10; ++i)
    {
      auto const line (random_single_line_string());

      oss << line << '\n';

      expected += prepend (i, line);

      BOOST_REQUIRE_EQUAL (oss.str(), expected);
    }
  }

  {
    auto const line (random_single_line_string());

    oss << line << '\n';

    expected += line + '\n';

    BOOST_REQUIRE_EQUAL (oss.str(), expected);
  }
}

BOOST_AUTO_TEST_CASE (last_line_is_kept_even_without_linebreak)
{
  std::ostringstream oss;
  auto const line (random_single_line_string());

  {
    fhg::util::ostream::redirect const redirecter
      ( oss
      , [&oss] (std::string const& got_line)
        {
          oss << got_line;
        }
      );

    oss << line;

    BOOST_REQUIRE (oss.str().empty());
  }

  BOOST_REQUIRE_EQUAL (oss.str(), line);
}

BOOST_AUTO_TEST_CASE (recursive_redirect)
{
  std::ostringstream oss;

  fhg::util::ostream::redirect const outer
    ( oss
    , [&oss] (std::string const& line_outer)
      {
        fhg::util::ostream::redirect const inner
          ( oss
          , [&oss] (std::string const& line_inner)
            {
              oss << "inner " << line_inner;
            }
          );

        oss << "outer " << line_outer << '\n';
      }
    );

  auto const line (random_single_line_string());

  oss << line << '\n';

  BOOST_REQUIRE_EQUAL (oss.str(), "inner outer " + line);
}

BOOST_AUTO_TEST_CASE (redirect_to_redirected_stream)
{
  std::ostringstream oss;

  fhg::util::ostream::redirect const cerr_to_oss
    ( std::cerr
    , [&oss] (std::string const& line)
      {
        oss << line;
      }
    );

  fhg::util::ostream::redirect const cout_to_cerr
    ( std::cout
    , [] (std::string const& line)
      {
        std::cerr << line << '\n';
      }
    );

  auto const line (random_single_line_string());

  std::cout << line << '\n';

  BOOST_REQUIRE_EQUAL (oss.str(), line);
}

BOOST_AUTO_TEST_CASE (prepender_function_of_line)
{
  std::ostringstream oss;

  fhg::util::ostream::prepend_line const prepender
    ( oss
    , [] (std::string const& line)
      {
        return fmt::format ("[{}]: ", line.size());
      }
    );

  std::string expected;

  auto const line (random_single_line_string());

  oss << line << '\n';

  BOOST_REQUIRE_EQUAL
    (oss.str(), fmt::format ("[{}]: {}\n", line.size(), line));
}

BOOST_AUTO_TEST_CASE (prepender_function_void)
{
  std::ostringstream oss;
  unsigned n (0);

  std::string expected;

  fhg::util::ostream::prepend_line const prepender
    (oss, [&n]() { return std::to_string (n++); });

  for (int i (0); i < 10; ++i)
  {
    auto const line (random_single_line_string());

    oss << line << '\n';

    expected += std::to_string (i) + line + '\n';

    BOOST_REQUIRE_EQUAL (oss.str(), expected);
  }
}

BOOST_AUTO_TEST_CASE (prepender_const)
{
  std::ostringstream oss;
  auto const prefix (random_single_line_string());

  std::string expected;

  fhg::util::ostream::prepend_line const prepender (oss, prefix);

  for (int i (0); i < 10; ++i)
  {
    auto const line (random_single_line_string());

    oss << line << '\n';

    expected += prefix + line + '\n';

    BOOST_REQUIRE_EQUAL (oss.str(), expected);
  }
}
