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
#include <iterator>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

//! \todo remove include
#include <util-generic/print_container.hpp>

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

  BOOST_DATA_TEST_CASE (remove_leaf_for_non_leaf_throws, from_to (2, 25), N)
  {
    auto const xs {randoms<std::vector<int>, unique_random> (N)};

    Forest<int> forest (chain (xs));

    for (std::size_t i (1); i < N; ++i)
    {
      fhg::util::testing::require_exception
        ( [&]
          {
            forest.remove_leaf (xs.at (i));
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            (str ( boost::format ("Forest::remove_leaf: ('%1%')")
                 % xs.at (i)
                 )
            )
          , std::invalid_argument ("Not a leaf.")
          )
        );
    }
  }

  BOOST_DATA_TEST_CASE (remove_root_for_non_root_throws, from_to (2, 25), N)
  {
    auto const xs {randoms<std::vector<int>, unique_random> (N)};

    Forest<int> forest (chain (xs));

    for (std::size_t i (0); i + 1 < N; ++i)
    {
      fhg::util::testing::require_exception
        ( [&]
          {
            forest.remove_root (xs.at (i));
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            (str ( boost::format ("Forest::remove_root: ('%1%')")
                 % xs.at (i)
                 )
            )
          , std::invalid_argument ("Not a root.")
          )
        );
    }
  }

  BOOST_DATA_TEST_CASE (remove_for_unknown_throws, from_to (0, 100), N)
  {
    auto const xs {randoms<std::vector<int>, unique_random> (N + 1)};

    Forest<int> forest (chain ({xs.cbegin(), std::prev (xs.end())}));

    fhg::util::testing::require_exception
      ( [&]
        {
          forest.remove_leaf (xs.back());
        }
      , fhg::util::testing::make_nested
        ( std::runtime_error
          (str ( boost::format ("Forest::remove_leaf: ('%1%')")
               % xs.back()
               )
          )
        , std::invalid_argument ("Unknown.")
        )
      );

    fhg::util::testing::require_exception
      ( [&]
        {
          forest.remove_root (xs.back());
        }
      , fhg::util::testing::make_nested
        ( std::runtime_error
          (str ( boost::format ("Forest::remove_root: ('%1%')")
               % xs.back()
               )
          )
        , std::invalid_argument ("Unknown.")
        )
      );
  }

  BOOST_DATA_TEST_CASE (remove_leaf_removes_leaf, from_to (0, 25), N)
  {
    auto const ns {randoms<std::vector<int>, unique_random> (N + 1)};
    auto const leaf {ns.front()};
    std::vector<int> const roots {ns.cbegin() + 1, ns.cend()};

    auto for_each_root
      ( [&] (auto&& callback)
        {
          std::for_each (roots.cbegin(), roots.cend(), callback);
        }
      );

    Forest<int> forest;
    forest.insert (leaf, {}, {});

    for_each_root ([&] (auto root) { forest.insert (root, {}, {leaf}); });

    auto check
      ( [&] (auto x, std::set<int> expected)
        {
          collectS<int> callback;

          forest.down (x, std::ref (callback));

          BOOST_REQUIRE_EQUAL_COLLECTIONS
            ( expected.begin(), expected.end()
            , callback._.begin(), callback._.end()
            );
        }
      );

    for_each_root ([&] (auto root) { check (root, {leaf, root}); });

    forest.remove_leaf (leaf);

    for_each_root ([&] (auto root) { check (root, {root}); });
  }

  BOOST_DATA_TEST_CASE (remove_root_removes_root, from_to (0, 25), N)
  {
    auto const ns {randoms<std::vector<int>, unique_random> (N + 1)};
    auto const root {ns.front()};
    typename Forest<int>::Children const leafs {ns.cbegin() + 1, ns.cend()};

    auto for_each_leaf
      ( [&] (auto&& callback)
        {
          std::for_each (leafs.cbegin(), leafs.cend(), callback);
        }
      );

    Forest<int> forest;

    for_each_leaf ([&] (auto leaf) { forest.insert (leaf, {}, {}); });

    forest.insert (root, {}, leafs);

    auto check
      ( [&] (auto leaf, std::set<int> expected)
        {
          collectS<int> callback;

          forest.up (leaf, std::ref (callback));

          BOOST_REQUIRE_EQUAL_COLLECTIONS
            ( expected.begin(), expected.end()
            , callback._.begin(), callback._.end()
            );
        }
      );

    for_each_leaf ([&] (auto leaf) { check (leaf, {leaf, root}); });

    forest.remove_root (root);

    for_each_leaf ([&] (auto leaf) { check (leaf, {leaf}); });
  }

  BOOST_AUTO_TEST_CASE (up_and_down)
  {
    // auto const xs {randoms<std::vector<int>, unique_random> (8)};
    // auto const n {xs.at (0)};
    // auto const m0 {xs.at (1)};
    // auto const m1 {xs.at (2)};
    // auto const s0 {xs.at (3)};
    // auto const s1 {xs.at (4)};
    // auto const s2 {xs.at (5)};
    // auto const s3 {xs.at (6)};
    // auto const gpu {xs.at (7)};

    std::string const n {"n"};
    std::string const m0 {"m0"};
    std::string const m1 {"m1"};
    std::string const s0 {"s0"};
    std::string const s1 {"s1"};
    std::string const s2 {"s2"};
    std::string const s3 {"s3"};
    std::string const gpu {"gpu"};

    Forest<std::string> forest;
    Forest<std::string>::Children cs0;
    Forest<std::string>::Children cs1;
    Forest<std::string>::Children cs2;
    Forest<std::string>::Children cs3;

    for (std::size_t c {0}; c < 8; ++c)
    {
      cs0.emplace (s0 + "-c" + std::to_string (c));
      cs1.emplace (s1 + "-c" + std::to_string (c));
      cs2.emplace (s2 + "-c" + std::to_string (c));
      cs3.emplace (s3 + "-c" + std::to_string (c));

      forest.insert (s0 + "-c" + std::to_string (c), {}, {});
      forest.insert (s1 + "-c" + std::to_string (c), {}, {});
      forest.insert (s2 + "-c" + std::to_string (c), {}, {});
      forest.insert (s3 + "-c" + std::to_string (c), {}, {});
    }

    forest.insert (s0, {}, cs0);
    forest.insert (s1, {}, cs1);
    forest.insert (s2, {}, cs2);
    forest.insert (s3, {}, cs3);
    forest.insert (m0, {}, {s0, s1});
    forest.insert (m1, {}, {s2, s3});
    forest.insert (n, {}, {m0, m1});
    forest.insert (gpu, {}, {s3});

    // auto check_down
    // { [&] (std::string root, std::set<std::string> expected)
    //   {
    //     collectS<std::string> callback;

    //     forest.down (root, std::ref (callback));

    //     BOOST_REQUIRE_EQUAL_COLLECTIONS
    //       ( expected.begin(), expected.end()
    //       , callback._.begin(), callback._.end()
    //       );
    //   }
    // };
    // auto check_up
    // { [&] (std::string root, std::set<std::string> expected)
    //   {
    //     collectS<std::string> callback;

    //     forest.up (root, std::ref (callback));

    //     BOOST_REQUIRE_EQUAL_COLLECTIONS
    //       ( expected.begin(), expected.end()
    //       , callback._.begin(), callback._.end()
    //       );
    //   }
    // };

    // check_down (n, {n, m0, m1, s0, s1, s2, s3});
    // check_down (m0, {m0, s0, s1});
    // check_down (m1, {m1, s2, s3});
    // check_down (s0, {s0});
    // check_down (s1, {s1});
    // check_down (s2, {s2});
    // check_down (s3, {s3});
    // check_down (gpu, {gpu, s3});

    // check_up (n, {n});
    // check_up (m0, {m0, n});
    // check_up (m1, {m1, n});
    // check_up (s0, {s0, m0, n});
    // check_up (s1, {s1, m0, n});
    // check_up (s2, {s2, m1, n});
    // check_up (s3, {s3, m1, n, gpu});
    // check_up (gpu, {gpu});

    forest.upward_combine_transform
      ( [] ( forest::Node<std::string> const& node
           , std::list<forest::Node<std::string> const*> const& children
           )
        {
          std::cout
            << "node " << node.first
            << " children "
            << fhg::util::print_container
               ( "{", ", ", "}", children
               , [&] (auto& os, auto& child) -> decltype (os)
                 {
                   return os << child->first;
                 }
               )
            << '\n'
            ;

          return node;
        }
      );
  }
}
