// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/hash/combined_hash.hpp>

#include <util-generic/testing/measure_average_time.hpp>
#include <util-generic/testing/printer/chrono.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

struct ntuple
{
  std::string entry[10];
};

FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( ::ntuple, t
  , t.entry[0], t.entry[1], t.entry[2], t.entry[3], t.entry[4]
  , t.entry[5], t.entry[6], t.entry[7], t.entry[8], t.entry[9]
  )

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (ntuple)
{
  random<std::string> gen;
  return { { gen(), gen(), gen(), gen(), gen(), gen(), gen(), gen(), gen()
           , gen()
           }
         };
}

BOOST_AUTO_TEST_CASE (combined_hash_is_as_fast_as_manual_combine)
{
  auto const tuples (fhg::util::testing::randoms<std::vector<ntuple>> (20000));

  constexpr auto const repetitions (100);
  constexpr auto const tolerance (5/*%*/);

  std::size_t const reference_accum
    ( [&]
      {
        std::size_t accum (0);
        for (auto const& tuple : tuples)
        {
          accum += std::hash<ntuple>() (tuple);
        }
        return accum;
      }()
    );

  auto const manual
    ( fhg::util::testing::measure_average_time<std::chrono::microseconds>
        ( [&]
          {
            std::size_t accum (0);
            for (auto const& tuple : tuples)
            {
              std::size_t seed (0);
              ::boost::hash_combine
                (seed, std::hash<std::string>() (tuple.entry[0]));
              ::boost::hash_combine
                (seed, std::hash<std::string>() (tuple.entry[1]));
              ::boost::hash_combine
                (seed, std::hash<std::string>() (tuple.entry[2]));
              ::boost::hash_combine
                (seed, std::hash<std::string>() (tuple.entry[3]));
              ::boost::hash_combine
                (seed, std::hash<std::string>() (tuple.entry[4]));
              ::boost::hash_combine
                (seed, std::hash<std::string>() (tuple.entry[5]));
              ::boost::hash_combine
                (seed, std::hash<std::string>() (tuple.entry[6]));
              ::boost::hash_combine
                (seed, std::hash<std::string>() (tuple.entry[7]));
              ::boost::hash_combine
                (seed, std::hash<std::string>() (tuple.entry[8]));
              ::boost::hash_combine
                (seed, std::hash<std::string>() (tuple.entry[9]));
              accum += seed;
            }
            BOOST_REQUIRE_EQUAL (accum, reference_accum);
          }
        , repetitions
        )
    );

  auto const loop
    ( fhg::util::testing::measure_average_time<std::chrono::microseconds>
        ( [&]
          {
            std::size_t accum (0);
            for (auto const& tuple : tuples)
            {
              std::size_t seed (0);
              for (std::size_t i (0); i < 10; ++i)
              {
                ::boost::hash_combine
                  (seed, std::hash<std::string>() (tuple.entry[i]));
              }
              accum += seed;
            }
            BOOST_REQUIRE_EQUAL (accum, reference_accum);
          }
        , repetitions
        )
    );

  auto const manual_combined
    ( fhg::util::testing::measure_average_time<std::chrono::microseconds>
        ( [&]
          {
            std::size_t accum (0);
            for (auto const& tuple : tuples)
            {
              accum += fhg::util::combined_hash
                ( tuple.entry[0]
                , tuple.entry[1]
                , tuple.entry[2]
                , tuple.entry[3]
                , tuple.entry[4]
                , tuple.entry[5]
                , tuple.entry[6]
                , tuple.entry[7]
                , tuple.entry[8]
                , tuple.entry[9]
                );
            }
            BOOST_REQUIRE_EQUAL (accum, reference_accum);
          }
        , repetitions
        )
    );

  auto const generated_combined
    ( fhg::util::testing::measure_average_time<std::chrono::microseconds>
        ( [&]
          {
            std::size_t accum (0);
            for (auto const& tuple : tuples)
            {
              accum += std::hash<ntuple>() (tuple);
            }
            BOOST_REQUIRE_EQUAL (accum, reference_accum);
          }
        , repetitions
        )
    );

  if (manual.count() < loop.count())
  {
    BOOST_CHECK_CLOSE ( static_cast<float> (manual.count())
                      , static_cast<float> (loop.count())
                      , tolerance
                      );
  }
  if (manual.count() < manual_combined.count())
  {
    BOOST_CHECK_CLOSE ( static_cast<float> (manual.count())
                      , static_cast<float> (manual_combined.count())
                      , tolerance
                      );
  }
  if (manual.count() < generated_combined.count())
  {
    BOOST_CHECK_CLOSE ( static_cast<float> (manual.count())
                      , static_cast<float> (generated_combined.count())
                      , tolerance
                      );
  }
}
