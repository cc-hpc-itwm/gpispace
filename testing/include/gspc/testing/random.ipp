#pragma once

#include <gspc/testing/random/bool.hpp>
#include <gspc/testing/random/char.hpp>
#include <gspc/testing/random/floating_point.hpp>
#include <gspc/testing/random/hard_integral_typedef.hpp>
#include <gspc/testing/random/integral.hpp>
#include <gspc/testing/random/string.hpp>

// \todo Not needed by this header, but a lot of test cases rely on
// this fixing a missing include that was resolved in 1.65. Add the
// missing include to all tests doing that, or bump Boost and remove
// this check.
#include <boost/version.hpp>
#if BOOST_VERSION < 106500
#include <boost/test/tree/test_unit.hpp>
#endif

#include <algorithm>
#include <unordered_set>



    namespace gspc::testing
    {
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
