#include <boost/test/unit_test.hpp>

#include <gspc/util/Forest.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/format.hpp>
#include <boost/noncopyable.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace gspc
{
  namespace util
  {
    using fhg::util::testing::random;
    using fhg::util::testing::randoms;
    using fhg::util::testing::unique_random;

    namespace
    {
      template<typename T>
        struct collect : boost::noncopyable
      {
        bool operator() (T x)
        {
          _.emplace_back (x);

          return true;
        }

        std::vector<T> _;
      };

      template<typename T>
        struct collectS : boost::noncopyable
      {
        bool operator() (T x)
        {
          _.emplace (x);

          return true;
        }

        std::set<T> _;
      };

      std::vector<std::size_t> from_to (std::size_t from, std::size_t to)
      {
        if (to < from)
        {
          throw std::invalid_argument
            (str (boost::format ("from_to: to %1% < from %2%") % to % from));
        }

        std::vector<std::size_t> xs (to - from);

        std::iota (xs.begin(), xs.end(), from);

        return xs;
      }
    }

    BOOST_DATA_TEST_CASE (loops_throw, from_to (1, 100), i)
    {
      auto const xs {randoms<std::vector<int>, unique_random> (i)};

      Forest<int> forest;

      for (std::size_t k {0}; k + 1 < i; ++k)
      {
        forest.insert (xs.at (k), xs.at (k + 1));
      }

      fhg::util::testing::require_exception
        ( [&]
          {
            forest.insert (xs.back(), xs.front());
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Forest::insert: (from %1%, to %2%)")
              % xs.back()
              % xs.front()
              ).str()
            )
          , std::invalid_argument ("Cycle.")
          )
        );
    }

    BOOST_DATA_TEST_CASE (diamonds_throw, from_to (3, 100), i)
    {
      auto const xs {randoms<std::vector<int>, unique_random> (i)};

      Forest<int> forest;

      for (std::size_t k {0}; k + 1 < i; ++k)
      {
        forest.insert (xs.at (k), xs.at (k + 1));
      }

      fhg::util::testing::require_exception
        ( [&]
          {
            forest.insert (xs.front(), xs.back());
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Forest::insert: (from %1%, to %2%)")
              % xs.front()
              % xs.back()
              ).str()
            )
          , std::invalid_argument ("Diamond.")
          )
        );
    }

    BOOST_DATA_TEST_CASE (duplicate_connections_throw, from_to (2, 100), i)
    {
      auto const xs {randoms<std::vector<std::size_t>, unique_random> (i)};

      Forest<std::size_t> forest;

      for (std::size_t k {0}; k + 1 < i; ++k)
      {
        forest.insert (xs.at (k), xs.at (k + 1));
      }

      auto const p {random<std::size_t>{}() % (i - 1)};

      fhg::util::testing::require_exception
        ( [&]
          {
            forest.insert (xs.at (p), xs.at (p + 1));
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Forest::insert: (from %1%, to %2%)")
              % xs.at (p)
              % xs.at (p + 1)
              ).str()
            )
          , std::invalid_argument ("Duplicate connection.")
          )
        );
    }

    BOOST_DATA_TEST_CASE
      (duplicate_UNSAFE_connections_throw, from_to (2, 100), i)
    {
      auto const xs {randoms<std::vector<std::size_t>, unique_random> (i)};

      Forest<std::size_t> forest;

      for (std::size_t k {0}; k + 1 < i; ++k)
      {
        forest.insert (xs.at (k), xs.at (k + 1));
      }

      auto const p {random<std::size_t>{}() % (i - 1)};

      fhg::util::testing::require_exception
        ( [&]
          {
            forest.UNSAFE_insert (xs.at (p), xs.at (p + 1));
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Forest::UNSAFE_insert: (from %1%, to %2%)")
              % xs.at (p)
              % xs.at (p + 1)
              ).str()
            )
          , std::invalid_argument ("Duplicate connection.")
          )
        );
    }

    BOOST_DATA_TEST_CASE
      (up_and_down_traverse_tree_in_opposite_order, from_to (2, 100), i)
    {
      auto const xs {randoms<std::vector<int>, unique_random> (i)};

      Forest<int> forest;

      for (std::size_t k {0}; k + 1 < i; ++k)
      {
        forest.insert (xs.at (k), xs.at (k + 1));
      }

      {
        collect<int> callback;

        forest.down (xs.front(), std::ref (callback));

        BOOST_REQUIRE_EQUAL_COLLECTIONS
          (xs.begin(), xs.end(), callback._.begin(), callback._.end());
      }

      {
        collect<int> callback;

        forest.up (xs.back(), std::ref (callback));

        BOOST_REQUIRE_EQUAL_COLLECTIONS
          (xs.rbegin(), xs.rend(), callback._.begin(), callback._.end());
      }
    }

    BOOST_AUTO_TEST_CASE (up_and_down_are_calling_back_at_least_once)
    {
      auto const id {fhg::util::testing::random<int>{}()};
      int c {0};

      auto callback
      { [&] (int x)
        {
          BOOST_REQUIRE_EQUAL (x, id);

          ++c;

          return true;
        }
      };

      Forest<int> forest;

      forest.down (id, callback);

      BOOST_REQUIRE_EQUAL (c, 1);

      forest.up (id, callback);

      BOOST_REQUIRE_EQUAL (c, 2);
    }

    BOOST_AUTO_TEST_CASE (remove_nonexisting_connection_throws)
    {
      auto const xs {randoms<std::vector<int>, unique_random> (3)};

      Forest<int> forest;

      auto const outer
        { std::runtime_error
          ( ( boost::format ("Forest::remove: (from %1%, to %2%)")
            % xs.at (0)
            % xs.at (1)
            ).str()
          )
          };

      fhg::util::testing::require_exception
        ( [&]
          {
            forest.remove (xs.at (0), xs.at (1));
          }
        , fhg::util::testing::make_nested
          ( outer
          , std::invalid_argument ("Connection does not exist: Missing from.")
          )
        );

      forest.insert (xs.at (0), xs.at (2));

      fhg::util::testing::require_exception
        ( [&]
          {
            forest.remove (xs.at (0), xs.at (1));
          }
        , fhg::util::testing::make_nested
          ( outer
          , std::invalid_argument ("Connection does not exist: Missing to.")
          )
        );
    }

    BOOST_AUTO_TEST_CASE (remove_connection_removes_connection)
    {
      auto const xs {randoms<std::vector<int>, unique_random> (4)};

      Forest<int> forest;

      auto check
      { [&] (std::vector<int> result, std::vector<std::size_t> indices)
        {
          std::vector<int> expected;

          for (auto index : indices)
          {
            expected.emplace_back (xs.at (index));
          }

          BOOST_REQUIRE_EQUAL_COLLECTIONS
            (expected.begin(), expected.end(), result.begin(), result.end());
        }
      };
      auto check_down
      { [&] (std::size_t root, std::vector<std::size_t> indices)
        {
          collect<int> callback;

          forest.down (xs.at (root), std::ref (callback));

          check (callback._, indices);
        }
      };
      auto check_up
      { [&] (std::size_t root, std::vector<std::size_t> indices)
        {
          collect<int> callback;

          forest.up (xs.at (root), std::ref (callback));

          check (callback._, indices);
        }
      };

      forest.insert (xs.at (0), xs.at (1))
        .insert (xs.at (1), xs.at (2))
        .insert (xs.at (2), xs.at (3))
        ;

      check_down (0, {0, 1, 2, 3});
      check_down (1, {1, 2, 3});
      check_down (2, {2, 3});
      check_down (3, {3});

      check_up (0, {0});
      check_up (1, {1, 0});
      check_up (2, {2, 1, 0});
      check_up (3, {3, 2, 1, 0});

      forest.remove (xs.at (1), xs.at (2));

      check_down (0, {0, 1});
      check_down (1, {1});
      check_down (2, {2, 3});
      check_down (3, {3});

      check_up (0, {0});
      check_up (1, {1, 0});
      check_up (2, {2});
      check_up (3, {3, 2});
    }

    BOOST_AUTO_TEST_CASE (remove_element_removes_all_connections)
    {
      auto const xs {randoms<std::vector<int>, unique_random> (6)};

      Forest<int> forest;

      auto check
      { [&] (std::vector<int> result, std::vector<std::size_t> indices)
        {
          std::vector<int> expected;

          for (auto index : indices)
          {
            expected.emplace_back (xs.at (index));
          }

          BOOST_REQUIRE_EQUAL_COLLECTIONS
            (expected.begin(), expected.end(), result.begin(), result.end());
        }
      };
      auto check_down
      { [&] (std::size_t root, std::vector<std::size_t> indices)
        {
          collect<int> callback;

          forest.down (xs.at (root), std::ref (callback));

          check (callback._, indices);
        }
      };
      auto check_up
      { [&] (std::size_t root, std::vector<std::size_t> indices)
        {
          collect<int> callback;

          forest.up (xs.at (root), std::ref (callback));

          check (callback._, indices);
        }
      };

      forest.insert (xs.at (0), xs.at (1))
            .insert (xs.at (1), xs.at (2))
            .insert (xs.at (2), xs.at (3))
            .insert (xs.at (4), xs.at (1))
            .insert (xs.at (1), xs.at (5))
            ;

      {
        std::set<int> expected {xs.at (1), xs.at (2), xs.at (3), xs.at (5)};

        collectS<int> callback;

        forest.down (xs.at (1), std::ref (callback));

        BOOST_REQUIRE_EQUAL_COLLECTIONS
          ( expected.begin(), expected.end()
          , callback._.begin(), callback._.end()
          );
      }

      {
        std::set<int> expected {xs.at (0), xs.at (1), xs.at (4)};

        collectS<int> callback;

        forest.up (xs.at (1), std::ref (callback));

        BOOST_REQUIRE_EQUAL_COLLECTIONS
          ( expected.begin(), expected.end()
          , callback._.begin(), callback._.end()
          );
      }

      forest.remove (xs.at (1));

      check_down (0, {0});
      check_down (1, {1});
      check_down (2, {2, 3});
      check_down (3, {3});
      check_down (4, {4});
      check_down (5, {5});

      check_up (0, {0});
      check_up (1, {1});
      check_up (2, {2});
      check_up (3, {3, 2});
      check_up (4, {4});
      check_up (5, {5});
    }

    BOOST_AUTO_TEST_CASE (up_and_down)
    {
      auto const xs {randoms<std::vector<int>, unique_random> (8)};
      auto const n {xs.at (0)};
      auto const m0 {xs.at (1)};
      auto const m1 {xs.at (2)};
      auto const s0 {xs.at (3)};
      auto const s1 {xs.at (4)};
      auto const s2 {xs.at (5)};
      auto const s3 {xs.at (6)};
      auto const gpu {xs.at (7)};

      Forest<int> forest;

      forest.insert (n, m0)
        .insert (n, m1)
        .insert (m0, s0)
        .insert (m0, s1)
        .insert (m1, s2)
        .insert (m1, s3)
        .insert (gpu, s3)
        ;

      auto check_down
      { [&] (int root, std::set<int> expected)
        {
          collectS<int> callback;

          forest.down (root, std::ref (callback));

          BOOST_REQUIRE_EQUAL_COLLECTIONS
            ( expected.begin(), expected.end()
            , callback._.begin(), callback._.end()
            );
        }
      };
      auto check_up
      { [&] (int root, std::set<int> expected)
        {
          collectS<int> callback;

          forest.up (root, std::ref (callback));

          BOOST_REQUIRE_EQUAL_COLLECTIONS
            ( expected.begin(), expected.end()
            , callback._.begin(), callback._.end()
            );
        }
      };

      check_down (n, {n, m0, m1, s0, s1, s2, s3});
      check_down (m0, {m0, s0, s1});
      check_down (m1, {m1, s2, s3});
      check_down (s0, {s0});
      check_down (s1, {s1});
      check_down (s2, {s2});
      check_down (s3, {s3});
      check_down (gpu, {gpu, s3});

      check_up (n, {n});
      check_up (m0, {m0, n});
      check_up (m1, {m1, n});
      check_up (s0, {s0, m0, n});
      check_up (s1, {s1, m0, n});
      check_up (s2, {s2, m1, n});
      check_up (s3, {s3, m1, n, gpu});
      check_up (gpu, {gpu});
    }

    BOOST_DATA_TEST_CASE (UNSAFE_insert_allows_for_loops, from_to (1, 100), i)
    {
      auto const xs {randoms<std::vector<int>, unique_random> (i)};

      Forest<int> forest;

      for (std::size_t k {0}; k + 1 < i; ++k)
      {
        forest.insert (xs.at (k), xs.at (k + 1));
      }

      forest.UNSAFE_insert (xs.back(), xs.front());

      Forest<int>::Ts seen;
      bool loop {false};

      forest.down ( xs.at (0)
                  , [&seen, &loop] (int x)
                    {
                      return !(loop = !seen.emplace (x).second);
                    }
                  );

      BOOST_REQUIRE (loop);
    }

    BOOST_AUTO_TEST_CASE (apply)
    {
      Forest<int> fi;
      fi.insert (0, 1);
      fi.insert (0, 2);
      fi.insert (2, 3);
      fi.insert (2, 4);
      fi.insert (4, 5);
      fi.insert (5, 6);
      fi.insert (6, 7);
      fi.insert (8, 7);
      fi.insert (9, 7);
      fi.insert (10, 9);
      fi.insert (7, 11);

      auto transform
        ( [] (int i)
          {
            std::cout << i << std::endl;

            return std::to_string (i);
          }
        );

      auto fs (fi.apply<std::string> (transform));
    }
  }
}
