#include <boost/test/unit_test.hpp>

#include <gspc/util/asynchronous.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/range/adaptor/uniqued.hpp>

#include <algorithm>
#include <atomic>
#include <numeric>
#include <type_traits>
#include <vector>

BOOST_AUTO_TEST_CASE (all_functions_are_executed)
{
  std::vector<unsigned long> const input
    { gspc::testing::randoms<std::remove_const<decltype (input)>::type>
        (gspc::testing::random<decltype (input)::size_type>()() % 1000)
    };

  std::atomic<decltype (input)::value_type> sum (0);

  gspc::util::asynchronous
    (input, [&sum] (decltype (input)::value_type const& x) { sum += x; });

  BOOST_REQUIRE_EQUAL
    ( sum.load()
    , std::accumulate ( input.begin(), input.end()
                      , static_cast<decltype (input)::value_type> (0)
                      )
    );
}

BOOST_AUTO_TEST_CASE (exceptions_are_transported)
{
  using T = int;

  T const number {gspc::testing::random<T>()()};

  gspc::testing::require_exception
    ( [number]()
      {
        gspc::util::asynchronous
          ( std::list<T> {number}
          , [] (T const& x)
            {
              throw std::runtime_error (std::to_string (x));
            }
          );
      }
    , std::runtime_error (std::to_string (number))
    );

  gspc::testing::require_exception
    ( [number]()
      {
        //! \note two times the same number to avoid ordering dependencies
        gspc::util::asynchronous
          ( std::list<T> {number, number}
          , [] (T const& x)
            {
              throw std::runtime_error (std::to_string (x));
            }
          );
      }
    , std::runtime_error
        (std::to_string (number) + '\n' + std::to_string (number))
    );
}

BOOST_AUTO_TEST_CASE (ranges)
{
  std::list<int> const list {0,1,2,2,3,4,4,5};

  std::atomic<int> sum (0);

  gspc::util::asynchronous
    ( list | ::boost::adaptors::uniqued
    , [&sum] (int const& x) { sum += x; }
    );

  BOOST_REQUIRE_EQUAL (sum.load(), 15);
}
