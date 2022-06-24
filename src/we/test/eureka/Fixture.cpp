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

#include <we/test/eureka/Fixture.hpp>

#include <algorithm>
#include <iterator>
#include <utility>

namespace gspc
{
  namespace we
  {
    namespace test
    {
      namespace eureka
      {
        unsigned int Fixture::random::sleep_time()
        {
          return _uint() % 3000u;
        }
        unsigned int Fixture::random::bit()
        {
          return _uint() % 2u;
        }
        auto Fixture::random::eureka_value() -> int
        {
          return _int();
        }
        auto Fixture::random::unique_task() -> Task
        {
          return _unique_eureka_group();
        }
        auto Fixture::random::unique_eureka_group() -> EurekaGroup
        {
          return _unique_task();
        }
        namespace
        {
          template<typename T, typename Gen>
            std::vector<T> generate (unsigned int k, Gen&& gen)
          {
            std::vector<T> xs (k);
            std::generate (std::begin (xs), std::end (xs), std::forward<Gen> (gen));
            return xs;
          }
        }
        auto Fixture::random::eureka_values (unsigned int k) -> std::vector<int>
        {
          return generate<int> (k, [&] { return eureka_value(); });
        }
        auto Fixture::random::unique_tasks (unsigned int k) -> std::vector<Task>
        {
          return generate<Task> (k, [&] { return unique_task(); });
        }
        auto Fixture::random::unique_eureka_groups (unsigned int k)
          -> std::vector<EurekaGroup>
        {
          return generate<EurekaGroup>
            (k, [&] { return unique_eureka_group(); });
        }
        std::string Fixture::random::identifier::operator()() const
        {
          return fhg::util::testing::random_identifier();
        }
        unsigned int Fixture::random::between
          (unsigned int min, unsigned int max)
        {
          return _uint (max, min);
        }
      }
    }
  }
}
