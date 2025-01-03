// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <logging/file_sink.hpp>
#include <logging/message.hpp>
#include <logging/stream_emitter.hpp>
#include <logging/test/message.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/split.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

namespace fhg
{
  namespace logging
  {
    namespace
    {
      //! \note file_sink always tries to subscribe, so the endpoint
      //! needs to be valid.
      stream_emitter const register_only_emitter;
      endpoint const register_only_endpoint
        (register_only_emitter.local_endpoint());
    }

    BOOST_AUTO_TEST_CASE (constructor_throws_if_file_not_writable)
    {
      ::boost::filesystem::path const non_existing_path
        ("/we/can't/write/to/this/because/missing");
      BOOST_REQUIRE (!::boost::filesystem::exists (non_existing_path));
      auto const target (non_existing_path / "parent");

      util::testing::require_exception
        ( [&]
          {
            file_sink sink ( register_only_endpoint
                           , target
                           , [] (std::ostream&, message const&) {}
                           , ::boost::none
                           );
          }
        , error::unable_to_write_to_file (target)
        );
    }

    namespace
    {
      struct public_file_sink : public file_sink
      {
        using file_sink::file_sink;
        using file_sink::dispatch_append;
      };

      std::vector<std::string> assemble_comparison_file
        (std::string sentinel, std::size_t count)
      {
        return std::vector<std::string> (count, sentinel);
      }

      std::string random_single_line_string()
      {
        return util::testing::random<std::string>{}
          (util::testing::random<std::string>::except ("\n"));
      }
    }

    BOOST_AUTO_TEST_CASE (formatter_is_invoked_per_message)
    {
      auto const emit_count (util::testing::random<std::size_t>{}() % 314);
      // '\n' is used as separator in file
      auto const sentinel (random_single_line_string());

      util::temporary_path const temporary_sink_dir;
      ::boost::filesystem::path const sink_path
        (static_cast<::boost::filesystem::path> (temporary_sink_dir) / "log");

      {
        public_file_sink sink
          ( register_only_endpoint
          , sink_path
          , [&] (std::ostream& os, message const&)
            {
              os << sentinel << '\n';
            }
          , ::boost::none
          );

        for (std::size_t i (0); i < emit_count; ++i)
        {
          sink.dispatch_append (util::testing::random<message>{}());
        }
      }

      BOOST_REQUIRE_EQUAL ( util::read_lines (sink_path)
                          , assemble_comparison_file (sentinel, emit_count)
                          );
    }

    namespace
    {
      void just_a_newline (std::ostream& os, message const&)
      {
        os << '\n';
      }
    }

    BOOST_AUTO_TEST_CASE (no_flushing_happens_if_none_given)
    {
      util::temporary_path const temporary_sink_dir;
      ::boost::filesystem::path const path
        (static_cast<::boost::filesystem::path> (temporary_sink_dir) / "log");

      auto const emits (util::testing::random<std::size_t>{}() % 134);

      {
        public_file_sink sink (register_only_endpoint, path, &just_a_newline, ::boost::none);

        for (std::size_t i (0); i < emits; ++i)
        {
          sink.dispatch_append (util::testing::random<message>{}());
          BOOST_REQUIRE_EQUAL (util::read_lines (path).size(), 0);
        }
      }

      BOOST_REQUIRE_EQUAL (util::read_lines (path).size(), emits);
    }

    BOOST_AUTO_TEST_CASE (flushing_can_be_overwritten_by_an_evil_formatter)
    {
      util::temporary_path const temporary_sink_dir;
      ::boost::filesystem::path const path
        (static_cast<::boost::filesystem::path> (temporary_sink_dir) / "log");

      auto const emits (util::testing::random<std::size_t>{}() % 134);

      {
        public_file_sink sink
          ( register_only_endpoint
          , path
          , [&] (std::ostream& os, message const&)
            {
              os << std::endl;
            }
          , ::boost::none
          );

        for (std::size_t i (0); i < emits; ++i)
        {
          sink.dispatch_append (util::testing::random<message>{}());
          BOOST_REQUIRE_EQUAL (util::read_lines (path).size(), i + 1);
        }
      }

      BOOST_REQUIRE_EQUAL (util::read_lines (path).size(), emits);
    }

    BOOST_AUTO_TEST_CASE (flush_interval_of_zero_always_flushes)
    {
      util::temporary_path const temporary_sink_dir;
      ::boost::filesystem::path const path
        (static_cast<::boost::filesystem::path> (temporary_sink_dir) / "log");

      auto const emits (util::testing::random<std::size_t>{}() % 134);

      {
        public_file_sink sink (register_only_endpoint, path, &just_a_newline, 0);

        for (std::size_t i (0); i < emits; ++i)
        {
          sink.dispatch_append (util::testing::random<message>{}());
          BOOST_REQUIRE_EQUAL (util::read_lines (path).size(), i + 1);
        }
      }

      BOOST_REQUIRE_EQUAL (util::read_lines (path).size(), emits);
    }

    BOOST_DATA_TEST_CASE
      ( flushing_happens_in_given_interval
      , ::boost::unit_test::data::make ({1ul, 2ul, 3ul, 5ul, 6ul, 13ul, 31ul})
      , interval
      )
    {
      util::temporary_path const temporary_sink_dir;
      ::boost::filesystem::path const path
        (static_cast<::boost::filesystem::path> (temporary_sink_dir) / "log");

      constexpr std::size_t const repetitions (13);
      std::size_t expected_lines (0);

      {
        public_file_sink sink (register_only_endpoint, path, &just_a_newline, interval);

        auto const emit
          ([&] { sink.dispatch_append (util::testing::random<message>{}()); });

        for (std::size_t repetition (0); repetition < repetitions; ++repetition)
        {
          for (unsigned long i (0); i + 1 < interval; ++i)
          {
            emit();
            BOOST_REQUIRE_EQUAL (util::read_lines (path).size(), expected_lines);
          }

          emit();
          expected_lines += interval;
          BOOST_REQUIRE_EQUAL (util::read_lines (path).size(), expected_lines);
        }

        for (unsigned long i (0); i + 1 < interval; ++i)
        {
          emit();
          BOOST_REQUIRE_EQUAL (util::read_lines (path).size(), expected_lines);
        }

        expected_lines += interval - 1;
      }

      BOOST_REQUIRE_EQUAL (util::read_lines (path).size(), expected_lines);
    }

    BOOST_AUTO_TEST_CASE (file_sink_actually_registers_and_receives)
    {
      auto const emit_count (util::testing::random<std::size_t>{}() % 314);
      // '\n' is used as separator in file
      auto const sentinel (random_single_line_string());

      stream_emitter emitter;

      util::temporary_path const temporary_sink_dir;
      ::boost::filesystem::path const path
        (static_cast<::boost::filesystem::path> (temporary_sink_dir) / "log");

      {
        file_sink const sink
          ( emitter.local_endpoint()
          , path
          , [&] (std::ostream& os, message const&)
            {
              os << sentinel << '\n';
            }
          , ::boost::none
          );

        for (std::size_t i (0); i < emit_count; ++i)
        {
          emitter.emit_message (message ("", ""));
        }
      }

      BOOST_REQUIRE_EQUAL ( util::read_lines (path)
                          , assemble_comparison_file (sentinel, emit_count)
                          );
    }
  }
}
