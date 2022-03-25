// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util-generic/testing/random.hpp>

#include <util-generic/join.hpp>
#include <util-generic/testing/random/state_bits.hpp>

#include <boost/test/framework.hpp>
#include <boost/test/utils/lazy_ostream.hpp>

#include <array>
#include <functional>
#include <iostream>
#include <mutex>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        namespace
        {
          struct seed_state_t
          {
            using seed_seq_type = std::seed_seq::result_type;
            static constexpr std::size_t seed_seq_size
              = state_bits<RandomNumberEngine>()
              / (sizeof (seed_seq_type) * CHAR_BIT);
            using seed_seq_data = std::array<seed_seq_type, seed_seq_size>;

            seed_seq_data seed_data = generate_seed();
            static seed_seq_data generate_seed()
            {
              //! Paste the seed sequence as a comma-separated
              //! sequence of seed_seq_size (2 for minstd_rand, 624
              //! for mt19937) seed_seq_type (uint_least32_t) integers
              //! here and uncomment if you want to fixate the seed:
              /*
#define FHG_UTIL_TESTING_OVERRIDE_SEED                                  \
              3714218433,1972383181
              */

#ifdef FHG_UTIL_TESTING_OVERRIDE_SEED
              return {{FHG_UTIL_TESTING_OVERRIDE_SEED}};
#else
              std::random_device source;
              seed_seq_data data;
              std::generate (data.begin(), data.end(), std::ref (source));
              return data;
#endif
            }

            void seed (RandomNumberEngine& engine) const
            {
              std::seed_seq seed_sequence (seed_data.begin(), seed_data.end());
              engine.seed (seed_sequence);
            }
          };

          seed_state_t& seed_state()
          {
            static seed_state_t _;
            return _;
          }
        }

        RandomNumberEngine& GLOBAL_random_engine()
        {
          static RandomNumberEngine engine;

          static std::once_flag was_seeded;
          std::call_once (was_seeded, [&] { seed_state().seed (engine); });

          // Context resets per test unit, so check if in a new unit
          // to avoid duplicate addition of context.
          static auto last_unit (::boost::unit_test::INV_TEST_UNIT_ID);
          auto const curr_unit
            (::boost::unit_test::framework::current_test_case_id());
          if (last_unit != curr_unit)
          {
            ::boost::unit_test::framework::add_context
              ( BOOST_TEST_LAZY_MSG
                  ( "random generator was seeded with: "
                  << join (seed_state().seed_data, ",")
                  )
              , true
              );
            last_unit = curr_unit;
          }

          return engine;
        }
      }
    }
  }
}
