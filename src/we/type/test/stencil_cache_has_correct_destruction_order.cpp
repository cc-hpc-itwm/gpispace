#include <boost/test/unit_test.hpp>

#include <gspc/StencilCache.hpp>
#include <test/gspc/stencil_cache/workflow/Neighborhood.hpp>
#include <we/type/stencil_cache.hpp>
#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/show.hpp>

#include <fhg/util/boost/program_options/generic.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/print_container.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>

namespace we
{
  namespace type
  {
    namespace
    {
      using Starter = int;
      using Who = int;
      using Next = int;

      struct mutual_exclusion
      {
        mutual_exclusion (Starter starter)
          : _who (starter)
        {}

        void exclusive (Who who, Next next, std::function<void()> what)
        {
          {
            std::unique_lock<std::mutex> lock {_guard};
            FHG_UTIL_FINALLY ([&] { _who = next; });
            _flip.wait (lock, [&] { return _who == who; });
            what();
          }
          _flip.notify_all();
        }

      private:
        Who _who;
        std::mutex _guard;
        std::condition_variable _flip;
      };
    }

    // using a 1-dimensional stencil with radius 0 implies that each
    // point depends on itself and only on itself

    // providing K slots for a grid of size (K+1) and never report any
    // slot as being prepared implies that the (K+1)-st allocation
    // will block

    // to destroy the cache in this moment shall not block

    BOOST_AUTO_TEST_CASE (stencil_cache_has_correct_destruction_order)
    {
      using fhg::util::print_container;
      using fhg::util::testing::random_identifier;
      using fhg::util::testing::random;
      using pnet::type::value::value_type;
      using pnet::type::value::poke;
      using Path = std::list<std::string>;
      using pnet::type::value::show;
      using test::gspc::stencil_cache::workflow::Neighborhood;
      namespace po = fhg::util::boost::program_options;

      auto peek
        ( [] (Path key, value_type const& x)
          {
            auto const value (pnet::type::value::peek (key, x));

            if (!value)
            {
              throw std::logic_error
                ( ( boost::format ("Missing binding for '%1%' in '%2%'.")
                  % print_container ("{", ", ", "}", key)
                  % show (x)
                  ).str()
                );
            }

            return value.get();
          }
        );

      po::option<po::existing_path> const option_neighbors {"neighbors", ""};

      boost::program_options::variables_map const vm
        ( fhg::util::boost::program_options::options
            ("stencil_cache_has_correct_destruction_order")
        . require (option_neighbors)
        . store_and_notify
          ( boost::unit_test::framework::master_test_suite().argc
          , boost::unit_test::framework::master_test_suite().argv
          )
        );

      auto const random_ulong (random<unsigned long>{});

      std::unordered_set<std::string> used_identifiers;

      std::function<std::string()> random_uniq_identifier
        ( [&]
          {
            auto const identifier (random_identifier());

            return used_identifiers.emplace (identifier).second
              ? identifier
              : random_uniq_identifier()
              ;
          }
        );

      auto const place_prepare (random_uniq_identifier());
      auto const place_ready (random_uniq_identifier());
      auto const place_neighbors (random_uniq_identifier());
      auto const place_neighbors_count (random_uniq_identifier());
      auto const memory_handle (random_identifier());
      auto const input_size (random_ulong());
      auto const block_size (1UL);

      auto const K (1 + random<unsigned int>{}() % 1000);

      auto const cK (gspc::stencil_cache::Coordinate {K});
      auto const cBegin (gspc::stencil_cache::Coordinate {0});
      auto const cEnd (cK + 1);
      auto const Kul (static_cast<unsigned long> (K));

      value_type neighbors;
      poke ("path", neighbors, option_neighbors.get_from (vm).string());
      poke ( "data", neighbors
           , we::type::bytearray (Neighborhood {cEnd, 1, 0}.data())
           );

      value_type memory;
      poke ("handle", memory, memory_handle);
      poke ("offset", memory, random_ulong());
      poke ("size", memory, Kul);

      value_type begin;
      poke ("value", begin, cBegin);

      value_type end;
      poke ("value", end, cEnd);

      expr::eval::context context;
      context.bind_and_discard_ref (Path {"place", "prepare"}, place_prepare);
      context.bind_and_discard_ref (Path {"place", "ready"}, place_ready);
      context.bind_and_discard_ref (Path {"place", "neighbors"}, place_neighbors);
      context.bind_and_discard_ref (Path {"place", "neighbors_count"}, place_neighbors_count);
      context.bind_and_discard_ref (Path {"memory"}, memory);
      context.bind_and_discard_ref (Path {"input_size"}, input_size);
      context.bind_and_discard_ref (Path {"block_size"}, block_size);
      context.bind_and_discard_ref (Path {"neighbors"}, neighbors);
      context.bind_and_discard_ref (Path {"begin"}, begin);
      context.bind_and_discard_ref (Path {"end"}, end);

      enum : int {ALLOC, PUT};

      mutual_exclusion sequential (Starter {ALLOC});
      expr::eval::context allocation_context;
      std::unordered_set<std::string> sync_callbacks;

      stencil_cache::PutToken put_token
        ( [&] (std::string place, value_type value)
          {
            if (place == place_prepare)
            {
              sequential.exclusive
                ( Who {PUT}
                , Next {ALLOC}
                , [&]
                  {
                    BOOST_REQUIRE_EQUAL
                      ( peek (Path {"coordinate"}, value)
                      , allocation_context.value (Path {"stencil"})
                      );

                    auto const memory_puts
                      ( boost::get<std::list<value_type>>
                          (peek (Path {"memory_put"}, value))
                      );

                    BOOST_REQUIRE_EQUAL (memory_puts.size(), 1);

                    BOOST_REQUIRE_EQUAL
                      ( peek (Path {"handle"}, memory_puts.front())
                      , value_type {memory_handle}
                      );
                    BOOST_REQUIRE_EQUAL
                      ( peek (Path {"size"}, memory_puts.front())
                      , value_type {input_size}
                      );
                  }
                );
            }
            else if (  place == place_neighbors
                    && sync_callbacks.emplace (place).second
                    )
            {
              BOOST_REQUIRE_EQUAL
                ( value
                , allocation_context.value (Path {"stencil"})
                );
            }
            else if (  place == place_neighbors_count
                    && sync_callbacks.emplace (place).second
                    )
            {
              BOOST_REQUIRE_EQUAL (value, value_type {1UL});
            }
            else
            {
              throw std::logic_error
                ( ( boost::format ("unexpected put_token (%1%, %2%)")
                  % place
                  % show (value)
                  ).str()
                );
            }
          }
        );

      stencil_cache stencil_cache {context, put_token};

      for (gspc::stencil_cache::Coordinate x (cBegin); x < cEnd; ++x)
      {
        sequential.exclusive
          ( Who {ALLOC}
          , Next {PUT}
          , [&]
            {
              allocation_context.bind_and_discard_ref
                (Path {"stencil", "value"}, x);

              sync_callbacks.clear();

              stencil_cache.alloc (allocation_context);

              BOOST_REQUIRE_EQUAL (sync_callbacks.size(), 2);
              BOOST_REQUIRE (sync_callbacks.count (place_neighbors));
              BOOST_REQUIRE (sync_callbacks.count (place_neighbors_count));
            }
          );
      }

      //! \note "long enough" to give the stencil cache time to
      //! retrieve the stencil to allocate from its internal queue and
      //! enter and block inside of the allocation routine, random to
      //! allow for _both_ interruptions to be executed/not executed
      std::this_thread::sleep_for
        (std::chrono::milliseconds (random<int>{}() % 10000));
    }
  }
}
