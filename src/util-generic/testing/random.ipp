// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#pragma once

#include <util-generic/testing/random/bool.hpp>
#include <util-generic/testing/random/char.hpp>
#include <util-generic/testing/random/floating_point.hpp>
#include <util-generic/testing/random/hard_integral_typedef.hpp>
#include <util-generic/testing/random/integral.hpp>
#include <util-generic/testing/random/string.hpp>

#include <boost/test/tree/observer.hpp>
#include <boost/test/tree/test_unit.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <algorithm>
#include <unordered_set>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        struct print_seed_on_failure_observer
          : public boost::unit_test::test_observer
        {
          print_seed_on_failure_observer();

          virtual void test_aborted() override;
          virtual void test_unit_aborted
            (boost::unit_test::test_unit const&) override;
        };

        BOOST_GLOBAL_FIXTURE (print_seed_on_failure_observer);
      }

      template<typename Container, typename Generator>
        Container randoms (std::size_t n, Generator&& generator)
      {
        Container container (n);
        std::generate (container.begin(), container.end(), generator);
        return container;
      }

      //! \note will busy-stall when no more unique values can be generated
      template<typename T, typename Generator>
        struct unique_random
      {
        T operator()()
        {
          while (true)
          {
            T value (_generator());
            if (_seen.emplace (value).second)
            {
              return value;
            }
          }
        }

      private:
        Generator _generator;
        std::unordered_set<T> _seen;
      };

      template<typename Container>
        Container unique_randoms (std::size_t count)
      {
        return randoms<Container>
          (count, unique_random<typename Container::value_type>{});
      }
    }
  }
}
