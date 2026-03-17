// Copyright (C) 2018-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/logging/file_sink.hpp>
#include <gspc/logging/message.hpp>
#include <gspc/logging/stream_emitter.hpp>
#include <test/logging/message.hpp>

#include <gspc/util/read_lines.hpp>
#include <gspc/util/split.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/vector.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>


  namespace gspc::logging
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
      std::filesystem::path const non_existing_path
        ("/we/can't/write/to/this/because/missing");
      BOOST_REQUIRE (!std::filesystem::exists (non_existing_path));
      auto const target (non_existing_path / "parent");

      gspc::testing::require_exception
        ( [&]
          {
            file_sink sink ( register_only_endpoint
                           , target
                           , [] (std::ostream&, message const&) {}
                           , std::nullopt
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
        return gspc::testing::random<std::string>{}
          (gspc::testing::random<std::string>::except ("\n"));
      }
    }

    BOOST_AUTO_TEST_CASE (formatter_is_invoked_per_message)
    {
      auto const emit_count (gspc::testing::random<std::size_t>{}() % 314);
      // '\n' is used as separator in file
      auto const sentinel (random_single_line_string());

      gspc::testing::temporary_path const temporary_sink_dir;
      auto const sink_path
        (static_cast<std::filesystem::path> (temporary_sink_dir) / "log");

      {
        public_file_sink sink
          ( register_only_endpoint
          , sink_path
          , [&] (std::ostream& os, message const&)
            {
              os << sentinel << '\n';
            }
          , std::nullopt
          );

        for (std::size_t i (0); i < emit_count; ++i)
        {
          sink.dispatch_append (gspc::testing::random<message>{}());
        }
      }

      BOOST_REQUIRE_EQUAL ( gspc::util::read_lines (sink_path)
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
      gspc::testing::temporary_path const temporary_sink_dir;
      auto const path
        (static_cast<std::filesystem::path> (temporary_sink_dir) / "log");

      auto const emits (gspc::testing::random<std::size_t>{}() % 134);

      {
        public_file_sink sink (register_only_endpoint, path, &just_a_newline, std::nullopt);

        for (std::size_t i (0); i < emits; ++i)
        {
          sink.dispatch_append (gspc::testing::random<message>{}());
          BOOST_REQUIRE_EQUAL (gspc::util::read_lines (path).size(), 0);
        }
      }

      BOOST_REQUIRE_EQUAL (gspc::util::read_lines (path).size(), emits);
    }

    BOOST_AUTO_TEST_CASE (flushing_can_be_overwritten_by_an_evil_formatter)
    {
      gspc::testing::temporary_path const temporary_sink_dir;
      auto const path
        (static_cast<std::filesystem::path> (temporary_sink_dir) / "log");

      auto const emits (gspc::testing::random<std::size_t>{}() % 134);

      {
        public_file_sink sink
          ( register_only_endpoint
          , path
          , [&] (std::ostream& os, message const&)
            {
              os << std::endl;
            }
          , std::nullopt
          );

        for (std::size_t i (0); i < emits; ++i)
        {
          sink.dispatch_append (gspc::testing::random<message>{}());
          BOOST_REQUIRE_EQUAL (gspc::util::read_lines (path).size(), i + 1);
        }
      }

      BOOST_REQUIRE_EQUAL (gspc::util::read_lines (path).size(), emits);
    }

    BOOST_AUTO_TEST_CASE (flush_interval_of_zero_always_flushes)
    {
      gspc::testing::temporary_path const temporary_sink_dir;
      auto const path
        (static_cast<std::filesystem::path> (temporary_sink_dir) / "log");

      auto const emits (gspc::testing::random<std::size_t>{}() % 134);

      {
        public_file_sink sink (register_only_endpoint, path, &just_a_newline, 0);

        for (std::size_t i (0); i < emits; ++i)
        {
          sink.dispatch_append (gspc::testing::random<message>{}());
          BOOST_REQUIRE_EQUAL (gspc::util::read_lines (path).size(), i + 1);
        }
      }

      BOOST_REQUIRE_EQUAL (gspc::util::read_lines (path).size(), emits);
    }

    BOOST_DATA_TEST_CASE
      ( flushing_happens_in_given_interval
      , ::boost::unit_test::data::make ({1ul, 2ul, 3ul, 5ul, 6ul, 13ul, 31ul})
      , interval
      )
    {
      gspc::testing::temporary_path const temporary_sink_dir;
      auto const path
        (static_cast<std::filesystem::path> (temporary_sink_dir) / "log");

      constexpr std::size_t const repetitions (13);
      std::size_t expected_lines (0);

      {
        public_file_sink sink (register_only_endpoint, path, &just_a_newline, interval);

        auto const emit
          ([&] { sink.dispatch_append (gspc::testing::random<message>{}()); });

        for (std::size_t repetition (0); repetition < repetitions; ++repetition)
        {
          for (unsigned long i (0); i + 1 < interval; ++i)
          {
            emit();
            BOOST_REQUIRE_EQUAL (gspc::util::read_lines (path).size(), expected_lines);
          }

          emit();
          expected_lines += interval;
          BOOST_REQUIRE_EQUAL (gspc::util::read_lines (path).size(), expected_lines);
        }

        for (unsigned long i (0); i + 1 < interval; ++i)
        {
          emit();
          BOOST_REQUIRE_EQUAL (gspc::util::read_lines (path).size(), expected_lines);
        }

        expected_lines += interval - 1;
      }

      BOOST_REQUIRE_EQUAL (gspc::util::read_lines (path).size(), expected_lines);
    }

    BOOST_AUTO_TEST_CASE (file_sink_actually_registers_and_receives)
    {
      auto const emit_count (gspc::testing::random<std::size_t>{}() % 314);
      // '\n' is used as separator in file
      auto const sentinel (random_single_line_string());

      stream_emitter emitter;

      gspc::testing::temporary_path const temporary_sink_dir;
      auto const path
        (static_cast<std::filesystem::path> (temporary_sink_dir) / "log");

      {
        file_sink const sink
          ( emitter.local_endpoint()
          , path
          , [&] (std::ostream& os, message const&)
            {
              os << sentinel << '\n';
            }
          , std::nullopt
          );

        for (std::size_t i (0); i < emit_count; ++i)
        {
          emitter.emit_message (message ("", ""));
        }
      }

      BOOST_REQUIRE_EQUAL ( gspc::util::read_lines (path)
                          , assemble_comparison_file (sentinel, emit_count)
                          );
    }
  }
