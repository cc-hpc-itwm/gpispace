// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <logging/ostream_redirect.hpp>
#include <logging/test/message.hpp>

#include <boost/test/unit_test.hpp>

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>

namespace fhg
{
  namespace logging
  {
    BOOST_AUTO_TEST_CASE (invokes_underlying_emitters_method)
    {
      auto const emit_count (util::testing::random<std::size_t>{}() % 314);

      struct
      {
        void emit_message (message const&)
        {
          ++called;
        }
        std::atomic<std::size_t> called = {0};
      } emitter;

      std::array<std::ostream*, 3> const ostream_choices
        {&std::cout, &std::cerr, &std::clog};
      auto const choice
        (util::testing::random<std::size_t>{}() % ostream_choices.size());

      {
        std::ostream& ostream (*ostream_choices[choice]);
        ostream_redirect const redirecter (ostream, emitter, "");

        for (std::size_t i (0); i < emit_count; ++i)
        {
          ostream << "\n";
        }
      }

      BOOST_REQUIRE_EQUAL (emitter.called, emit_count);
    }

    BOOST_AUTO_TEST_CASE (emits_with_given_message_without_newline)
    {
      auto const content (util::testing::random_string_without ("\n"));

      struct
      {
        void emit_message (message const& message)
        {
          content = message._content;
        }
        std::string content;
      } emitter;

      {
        ostream_redirect const redirecter (std::clog, emitter, "");
        std::clog << content << "\n";
      }

      BOOST_REQUIRE_EQUAL (emitter.content, content);
    }

    BOOST_AUTO_TEST_CASE (emits_with_given_category)
    {
      auto const category (util::testing::random<std::string>{}());

      struct
      {
        void emit_message (message const& message)
        {
          category = message._category;
        }
        std::string category;
      } emitter;

      {
        ostream_redirect const redirecter (std::clog, emitter, category);
        std::clog << "\n";
      }

      BOOST_REQUIRE_EQUAL (emitter.category, category);
    }
  }
}
