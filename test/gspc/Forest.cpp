#include <boost/test/unit_test.hpp>

#include <gspc/Forest.hpp>

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
  using fhg::util::testing::random;
  using fhg::util::testing::randoms;
  using fhg::util::testing::unique_random;

  namespace
  {
    template<typename T>
      struct collect : boost::noncopyable
    {
      void operator() (forest::Node<T> const& x)
      {
        _.emplace_back (x.first);
      }

      std::vector<T> _;
    };

    template<typename T>
      struct collectS : boost::noncopyable
    {
      void operator() (forest::Node<T> const& x)
      {
        _.emplace (x.first);
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

    // xs[0] <- xs[1] <- ... <- xs[xs.size()-1]
    void chain (Forest<int>& forest, std::vector<int> xs)
    {
      for (std::size_t i {0}; i + 1 < xs.size(); ++i)
      {
        forest.insert (xs.at (i + 1), {}, {xs.at (i)});
      }
    }
    Forest<int> chain (std::vector<int> xs)
    {
      Forest<int> forest;

      if (!xs.empty())
      {
        forest.insert (xs.front(), {}, {});

        chain (forest, xs);
      }

      return forest;
    }
  }

  BOOST_DATA_TEST_CASE
    (loop_and_duplicate_connection_throws, from_to (0, 25), N)
  {
    auto const xs {randoms<std::vector<int>, unique_random> (N)};

    Forest<int> forest (chain (xs));

    for (std::size_t a {0}; a < N; ++a)
    {
      for (std::size_t b {a}; b < N; ++b)
      {
        fhg::util::testing::require_exception
          ( [&]
            {
              forest.insert (xs.at (a), {}, {xs.at (b)});
            }
          , fhg::util::testing::make_nested
            ( std::runtime_error
              ( ( boost::format ("Forest::insert: (from '%1%', to {'%2%'})")
                % xs.at (a)
                % xs.at (b)
                ).str()
              )
            , std::invalid_argument ("Duplicate.")
            )
          );

        fhg::util::testing::require_exception
          ( [&]
            {
              forest.insert (xs.at (b), {}, {xs.at (a)});
            }
          , fhg::util::testing::make_nested
            ( std::runtime_error
              ( ( boost::format ("Forest::insert: (from '%1%', to {'%2%'})")
                % xs.at (b)
                % xs.at (a)
                ).str()
              )
            , std::invalid_argument ("Duplicate.")
            )
          );
      }
    }
  }

    BOOST_DATA_TEST_CASE ( diamonds_throw
                         , from_to (1, 25) * from_to (1, 25)
                         , N
                         , M
                         )
    {
      // ys[ys.size()-1] -> ... -> ys[0] -> C <- xs[0] <- ... <- xs[xs.size()-1]
      auto const ns {randoms<std::vector<int>, unique_random> (N + M + 1 + 1)};

      std::vector<int> const xs {  ns.cbegin() + 0    , ns.cbegin() + N};
      int              const C  {*(ns.cbegin() + N)};
      std::vector<int> const ys {  ns.cbegin() + N + 1, ns.cbegin() + N + M + 1};

      int              const D  {ns.back()};

      Forest<int> forest;
      forest.insert (C, {}, {});
      forest.insert (xs.front(), {}, {C});
      forest.insert (ys.front(), {}, {C});
      chain (forest, xs);
      chain (forest, ys);

      Forest<int>::Children const children {xs.back(), ys.back()};

      fhg::util::testing::require_exception
        ( [&]
          {
            forest.insert (D, {}, children);
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Forest::insert: (from '%1%', to %2%)")
              % D
              % fhg::util::print_container ("{'", "', '", "'}", children)
              ).str()
            )
          , std::invalid_argument ("Diamond.")
          )
        );
    }

    BOOST_AUTO_TEST_CASE (specific_diamond_throws)
    {
      // |             a         a   |
      // |            / \       / \  |
      // | b   d  +  b   d  =  b   d |
      // |  \ /                 \ /  |
      // |   c                   c   |
      Forest<char> forest;
      forest.insert ('c', {}, {});
      forest.insert ('b', {}, {'c'});
      forest.insert ('d', {}, {'c'});
      decltype (forest)::Children const children {'b','d'};

      fhg::util::testing::require_exception
        ( [&]
          {
            forest.insert ('a', {}, children);
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            (str ( boost::format ("Forest::insert: (from 'a', to %1%)")
                 % fhg::util::print_container ("{'", "', '", "'}", children)
                 )
            )
          , std::invalid_argument ("Diamond.")
          )
        );
    }

    BOOST_DATA_TEST_CASE
      (up_and_down_traverse_tree_in_opposite_order, from_to (2, 100), N)
    {
      auto const xs {randoms<std::vector<int>, unique_random> (N)};

      Forest<int> forest (chain (xs));

      {
        collect<int> callback;

        forest.up (xs.front(), std::ref (callback));

        BOOST_REQUIRE_EQUAL_COLLECTIONS
          (xs.begin(), xs.end(), callback._.begin(), callback._.end());
      }

      {
        collect<int> callback;

        forest.down (xs.back(), std::ref (callback));

        BOOST_REQUIRE_EQUAL_COLLECTIONS
          (xs.rbegin(), xs.rend(), callback._.begin(), callback._.end());
      }
    }

    BOOST_AUTO_TEST_CASE (up_and_down_are_calling_back_at_least_once)
    {
      auto const id {fhg::util::testing::random<int>{}()};
      int c {0};

      auto callback
      { [&] (forest::Node<int> x)
        {
          BOOST_REQUIRE_EQUAL (x.first, id);

          ++c;
        }
      };

      Forest<int> forest;
      forest.insert (id, {}, {});

      forest.down (id, callback);

      BOOST_REQUIRE_EQUAL (c, 1);

      forest.up (id, callback);

      BOOST_REQUIRE_EQUAL (c, 2);
    }

  //! \todo
  //   BOOST_AUTO_TEST_CASE (remove_nonexisting_connection_throws)
  //   {
  //     auto const xs {randoms<std::vector<int>, unique_random> (3)};

  //     Forest<int> forest;

  //     auto const outer
  //       { std::runtime_error
  //         ( ( boost::format ("Forest::remove: (from %1%, to %2%)")
  //           % xs.at (0)
  //           % xs.at (1)
  //           ).str()
  //         )
  //         };

  //     fhg::util::testing::require_exception
  //       ( [&]
  //         {
  //           forest.remove (xs.at (0), xs.at (1));
  //         }
  //       , fhg::util::testing::make_nested
  //         ( outer
  //         , std::invalid_argument ("Connection does not exist: Missing from.")
  //         )
  //       );

  //     forest.insert (xs.at (0), xs.at (2));

  //     fhg::util::testing::require_exception
  //       ( [&]
  //         {
  //           forest.remove (xs.at (0), xs.at (1));
  //         }
  //       , fhg::util::testing::make_nested
  //         ( outer
  //         , std::invalid_argument ("Connection does not exist: Missing to.")
  //         )
  //       );
  //   }

  //   BOOST_AUTO_TEST_CASE (remove_connection_removes_connection)
  //   {
  //     auto const xs {randoms<std::vector<int>, unique_random> (4)};

  //     Forest<int> forest;

  //     auto check
  //     { [&] (std::vector<int> result, std::vector<std::size_t> indices)
  //       {
  //         std::vector<int> expected;

  //         for (auto index : indices)
  //         {
  //           expected.emplace_back (xs.at (index));
  //         }

  //         BOOST_REQUIRE_EQUAL_COLLECTIONS
  //           (expected.begin(), expected.end(), result.begin(), result.end());
  //       }
  //     };
  //     auto check_down
  //     { [&] (std::size_t root, std::vector<std::size_t> indices)
  //       {
  //         collect<int> callback;

  //         forest.down (xs.at (root), std::ref (callback));

  //         check (callback._, indices);
  //       }
  //     };
  //     auto check_up
  //     { [&] (std::size_t root, std::vector<std::size_t> indices)
  //       {
  //         collect<int> callback;

  //         forest.up (xs.at (root), std::ref (callback));

  //         check (callback._, indices);
  //       }
  //     };

  //     forest.insert (xs.at (0), xs.at (1))
  //       .insert (xs.at (1), xs.at (2))
  //       .insert (xs.at (2), xs.at (3))
  //       ;

  //     check_down (0, {0, 1, 2, 3});
  //     check_down (1, {1, 2, 3});
  //     check_down (2, {2, 3});
  //     check_down (3, {3});

  //     check_up (0, {0});
  //     check_up (1, {1, 0});
  //     check_up (2, {2, 1, 0});
  //     check_up (3, {3, 2, 1, 0});

  //     forest.remove (xs.at (1), xs.at (2));

  //     check_down (0, {0, 1});
  //     check_down (1, {1});
  //     check_down (2, {2, 3});
  //     check_down (3, {3});

  //     check_up (0, {0});
  //     check_up (1, {1, 0});
  //     check_up (2, {2});
  //     check_up (3, {3, 2});
  //   }

  //   BOOST_AUTO_TEST_CASE (remove_element_removes_all_connections)
  //   {
  //     auto const xs {randoms<std::vector<int>, unique_random> (6)};

  //     Forest<int> forest;

  //     auto check
  //     { [&] (std::vector<int> result, std::vector<std::size_t> indices)
  //       {
  //         std::vector<int> expected;

  //         for (auto index : indices)
  //         {
  //           expected.emplace_back (xs.at (index));
  //         }

  //         BOOST_REQUIRE_EQUAL_COLLECTIONS
  //           (expected.begin(), expected.end(), result.begin(), result.end());
  //       }
  //     };
  //     auto check_down
  //     { [&] (std::size_t root, std::vector<std::size_t> indices)
  //       {
  //         collect<int> callback;

  //         forest.down (xs.at (root), std::ref (callback));

  //         check (callback._, indices);
  //       }
  //     };
  //     auto check_up
  //     { [&] (std::size_t root, std::vector<std::size_t> indices)
  //       {
  //         collect<int> callback;

  //         forest.up (xs.at (root), std::ref (callback));

  //         check (callback._, indices);
  //       }
  //     };

  //     forest.insert (xs.at (0), xs.at (1))
  //           .insert (xs.at (1), xs.at (2))
  //           .insert (xs.at (2), xs.at (3))
  //           .insert (xs.at (4), xs.at (1))
  //           .insert (xs.at (1), xs.at (5))
  //           ;

  //     {
  //       std::set<int> expected {xs.at (1), xs.at (2), xs.at (3), xs.at (5)};

  //       collectS<int> callback;

  //       forest.down (xs.at (1), std::ref (callback));

  //       BOOST_REQUIRE_EQUAL_COLLECTIONS
  //         ( expected.begin(), expected.end()
  //         , callback._.begin(), callback._.end()
  //         );
  //     }

  //     {
  //       std::set<int> expected {xs.at (0), xs.at (1), xs.at (4)};

  //       collectS<int> callback;

  //       forest.up (xs.at (1), std::ref (callback));

  //       BOOST_REQUIRE_EQUAL_COLLECTIONS
  //         ( expected.begin(), expected.end()
  //         , callback._.begin(), callback._.end()
  //         );
  //     }

  //     forest.remove (xs.at (1));

  //     check_down (0, {0});
  //     check_down (1, {1});
  //     check_down (2, {2, 3});
  //     check_down (3, {3});
  //     check_down (4, {4});
  //     check_down (5, {5});

  //     check_up (0, {0});
  //     check_up (1, {1});
  //     check_up (2, {2});
  //     check_up (3, {3, 2});
  //     check_up (4, {4});
  //     check_up (5, {5});
  //   }

  //   BOOST_AUTO_TEST_CASE (up_and_down)
  //   {
  //     auto const xs {randoms<std::vector<int>, unique_random> (8)};
  //     auto const n {xs.at (0)};
  //     auto const m0 {xs.at (1)};
  //     auto const m1 {xs.at (2)};
  //     auto const s0 {xs.at (3)};
  //     auto const s1 {xs.at (4)};
  //     auto const s2 {xs.at (5)};
  //     auto const s3 {xs.at (6)};
  //     auto const gpu {xs.at (7)};

  //     Forest<int> forest;

  //     forest.insert (n, m0)
  //       .insert (n, m1)
  //       .insert (m0, s0)
  //       .insert (m0, s1)
  //       .insert (m1, s2)
  //       .insert (m1, s3)
  //       .insert (gpu, s3)
  //       ;

  //     auto check_down
  //     { [&] (int root, std::set<int> expected)
  //       {
  //         collectS<int> callback;

  //         forest.down (root, std::ref (callback));

  //         BOOST_REQUIRE_EQUAL_COLLECTIONS
  //           ( expected.begin(), expected.end()
  //           , callback._.begin(), callback._.end()
  //           );
  //       }
  //     };
  //     auto check_up
  //     { [&] (int root, std::set<int> expected)
  //       {
  //         collectS<int> callback;

  //         forest.up (root, std::ref (callback));

  //         BOOST_REQUIRE_EQUAL_COLLECTIONS
  //           ( expected.begin(), expected.end()
  //           , callback._.begin(), callback._.end()
  //           );
  //       }
  //     };

  //     check_down (n, {n, m0, m1, s0, s1, s2, s3});
  //     check_down (m0, {m0, s0, s1});
  //     check_down (m1, {m1, s2, s3});
  //     check_down (s0, {s0});
  //     check_down (s1, {s1});
  //     check_down (s2, {s2});
  //     check_down (s3, {s3});
  //     check_down (gpu, {gpu, s3});

  //     check_up (n, {n});
  //     check_up (m0, {m0, n});
  //     check_up (m1, {m1, n});
  //     check_up (s0, {s0, m0, n});
  //     check_up (s1, {s1, m0, n});
  //     check_up (s2, {s2, m1, n});
  //     check_up (s3, {s3, m1, n, gpu});
  //     check_up (gpu, {gpu});
  //   }
}
