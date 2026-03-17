#include <gspc/testing/require_container_is_permutation.hpp>

#include <gspc/testing/printer/list.hpp>
#include <gspc/testing/printer/map.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/printer/set.hpp>
#include <gspc/testing/printer/unordered_map.hpp>
#include <gspc/testing/printer/unordered_multimap.hpp>
#include <gspc/testing/printer/unordered_set.hpp>
#include <gspc/testing/printer/vector.hpp>
#include <gspc/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <cstddef>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>



    namespace gspc::testing
    {
      BOOST_AUTO_TEST_CASE (empty_sequences_are_equal)
      {
        GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION
          (std::list<int>{}, std::list<int>{});
        GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION
          (std::set<float>{}, std::set<float>{});
      }

      namespace
      {
        void fill_with_random (std::set<float>& container)
        {
          unique_random<float> keys;
          for (int i (0), count (random<int>{} (1000, 100)); i < count; ++i)
          {
            container.emplace (keys());
          }
        }
      }

      BOOST_AUTO_TEST_CASE (comparing_to_self_is_equal)
      {
        std::set<float> container;

        fill_with_random (container);

        GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION
          (container, container);
      }

      namespace
      {
        template<typename Container>
          Container shuffled (Container c)
        {
          std::shuffle (c.begin(), c.end(), detail::GLOBAL_random_engine());
          return c;
        }
      }

      BOOST_AUTO_TEST_CASE (comparing_to_permutation_is_equal_random_ints)
      {
        auto const base
          (unique_randoms<std::vector<int>> (random<std::size_t>{} (1000, 100)));

        GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION
          (base, shuffled (base));
      }

      BOOST_AUTO_TEST_CASE (comparing_to_permutation_is_equal_anagram_string)
      {
        std::string const a
          ("NAHTRIHE CCUNDE GAHINNEVE RAHTUNIN, ZEHGE SSURKLACH, ZUNNUS");
        std::string const b
          ("CARL GUSTAV IUNG, IN KUESNACH, IAHR NEUNZEHNHUNDERTSECHZEHN");

        GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (a, b);
      }

      namespace
      {
        template<typename Key, typename Value>
          void fill_with_random ( std::map<Key, Value>& ordered
                                , std::unordered_map<Key, Value>& unordered
                                )
        {
          unique_random<Key> keys;
          for (int i (0), count (random<int>{} (1000, 100)); i < count; ++i)
          {
            unordered.emplace
              (*ordered.emplace (keys(), random<Value>{}()).first);
          }
        }
      }

      BOOST_AUTO_TEST_CASE (comparing_to_permutation_is_equal_ordered_unordered)
      {
        std::map<std::string, int> ordered;
        std::unordered_map<std::string, int> unordered;

        fill_with_random (ordered, unordered);

        GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (ordered, unordered);
      }

      namespace
      {
        template<typename Key, typename Value>
          void fill_with_random ( std::multimap<Key, Value> ordered
                                , std::unordered_multimap<Key, Value> unordered
                                )
        {
          auto const unique_keys
            (unique_randoms<std::vector<Key>> (random<std::size_t>{} (50, 5)));
          auto const keys
            ( [&]
              {
                return unique_keys.at
                  (random<std::size_t>{} (unique_keys.size() - 1));
              }
            );
          for (int i (0), count (random<int>{} (1000, 100)); i < count; ++i)
          {
            unordered.emplace (*ordered.emplace (keys(), random<Value>{}()));
          }
        }
      }

      BOOST_AUTO_TEST_CASE
        (comparing_to_permutation_is_equal_ordered_unordered_multi)
      {
        std::multimap<float, std::string> ordered;
        std::unordered_multimap<float, std::string> unordered;

        fill_with_random (ordered, unordered);

        GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (ordered, unordered);
      }
    }
