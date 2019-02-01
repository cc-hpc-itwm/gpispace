#include <boost/test/unit_test.hpp>

#include <gspc/StencilCache.hpp>

#include <fhg/util/boost/program_options/generic.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <util-generic/latch.hpp>
#include <util-generic/ostream/prefix_per_line.hpp>
#include <util-generic/threadsafe_queue.hpp>
#include <util-generic/timer/application.hpp>

#include <boost/format.hpp>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <list>
#include <stdexcept>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

namespace test
{
  namespace gspc
  {
    namespace stencil_cache
    {
      using Coordinate = long;
      using Point = std::pair<Coordinate, Coordinate>;
      using Slot = unsigned long;
      using Stencil = long;
      using Counter = unsigned int;

      Point to_point (Coordinate Y, Stencil);
      Coordinate to_coordinate (Coordinate Y, Point);
      std::list<Coordinate> neighbors
        (Coordinate X, Coordinate Y, Coordinate R, Point);

      Point to_point (Coordinate Y, Stencil c)
      {
        auto const x (c / Y);
        auto const y (c % Y);

        return Point {x, y};
      }

      Coordinate to_coordinate (Coordinate Y, Point point)
      {
        auto const x (point.first);
        auto const y (point.second);

        return y + Y * x;
      }

      std::list<Coordinate> neighbors
        (Coordinate X, Coordinate Y, Coordinate R, Point p)
      {
        std::list<Coordinate> points;

        for (Coordinate dx {-R}; dx < R + Coordinate {1}; ++dx)
        {
          auto const x (p.first + dx);

          if (0 <= x && x < X)
          {
            for (Coordinate dy {-R}; dy < R + Coordinate {1}; ++dy)
            {
              auto const y (p.second + dy);

              if (0 <= y && y < Y)
              {
                points.emplace_back (to_coordinate (Y, Point {x, y}));
              }
            }
          }
        }

        return points;
      }

      using SCache = ::gspc::StencilCache<Stencil, Coordinate, Slot, Counter>;

      SCache::InputEntries input_entries
        ( Coordinate X
        , Coordinate Y
        , Coordinate R
        , Slot M
        , std::size_t& max
        , std::size_t& sum
        )
      {
        SCache::InputEntries input_entries;

        max = sum = 0;

        for (Coordinate x {0}; x < X; ++x)
        {
          for (Coordinate y {0}; y < Y; ++y)
          {
            auto const point (Point {x, y});
            auto const ns (neighbors (X, Y, R, point));

            sum += ns.size();
            max = std::max (max, ns.size());

            for (auto const neighbor : ns)
            {
              input_entries[neighbor].increment();
            }
          }
        }

        if (M < max)
        {
          throw std::invalid_argument
            ( ( boost::format ("Not enough memory (%1% < %2%)")
              % M
              % max
              ).str()
            );
        }

        return input_entries;
      }

      namespace option
      {
        namespace po = fhg::util::boost::program_options;

        po::option<po::positive_integral<Coordinate>> const X {"X", "X"};
        po::option<po::positive_integral<Coordinate>> const Y {"Y", "Y"};

        po::option<po::positive_integral<Coordinate>> const R {"R", "R"};

        po::option<po::positive_integral<Slot>> const M {"M", "M"};

        po::option<po::positive_integral<std::size_t>> const C {"C", "C"};
        po::option<po::positive_integral<std::size_t>> const L {"L", "L"};
      }

      BOOST_AUTO_TEST_CASE (stencil2D)
      {
        boost::program_options::variables_map const vm
          ( fhg::util::boost::program_options::options ("stencil2D")
          . add ( fhg::util::boost::program_options::options ("grid")
                . require (option::X)
                . require (option::Y)
                )
          . add ( fhg::util::boost::program_options::options ("stencil radius")
                . require (option::R)
                )
          . add ( fhg::util::boost::program_options::options ("memory")
                . require (option::M)
                )
          . add ( fhg::util::boost::program_options::options ("#threads")
                . require (option::C)
                . require (option::L)
                )
          . store_and_notify
            ( boost::unit_test::framework::master_test_suite().argc
            , boost::unit_test::framework::master_test_suite().argv
            )
          );

        auto const X (option::X.get_from (vm));
        auto const Y (option::Y.get_from (vm));
        auto const R (option::R.get_from (vm));
        auto const M (option::M.get_from (vm));
        auto const Parallel_Compute (option::C.get_from (vm));
        auto const Parallel_Load (option::L.get_from (vm));

        auto const parameter
          ( ( boost::format ("X %1% Y %2% R %3% M %4% C %5% L %6%")
            % X
            % Y
            % R
            % M
            % Parallel_Compute
            % Parallel_Load
            )
          );
        fhg::util::default_application_timer out
          {(boost::format ("Stencil2D %1%") % parameter).str()};

        using Prepare = std::pair<Slot, Coordinate>;
        using QPrepare = fhg::util::interruptible_threadsafe_queue<Prepare>;
        using Assignment = SCache::Assignment;
        using Ready = std::pair<Stencil, Assignment>;
        using QReady = fhg::util::interruptible_threadsafe_queue<Ready>;
        using QUsed = fhg::util::interruptible_threadsafe_queue<Coordinate>;
        using QPrepared = fhg::util::interruptible_threadsafe_queue<Coordinate>;

        QPrepare prepare;
        QReady ready;
        QUsed used;
        QPrepared prepared;

        //! \note unsynchronized access!
        std::vector<Coordinate> memory (M);

        out.section ("Preprocess stencil");

        std::size_t sum_neighbors {0};
        std::size_t max_neighbors {0};

        out.section ("Create cache");

        SCache cache ( input_entries
                         (X, Y, R, M, max_neighbors, sum_neighbors)
                     , [&prepare] (Slot slot, Coordinate c)
                       {
                         prepare.put (Prepare {slot, c});
                       }
                     , [&ready] (Stencil stencil, Assignment assignment)
                       {
                         ready.put (Ready {stencil, assignment});
                       }
                     , M
                     );

        std::atomic<std::size_t> number_of_load {0};
        std::unordered_set<Stencil> computed;
        std::mutex _guard_computed;

        fhg::util::latch computations (X * Y);

        std::vector<std::thread> async_load;
        std::vector<std::thread> async_compute;

        fhg::util::latch async_load_running {Parallel_Load};
        fhg::util::latch async_compute_running {Parallel_Compute};
        fhg::util::latch async_used_running {1};
        fhg::util::latch async_prepared_running {1};

        std::vector<Ready> ERROR_duplicated_computation;
        std::vector<Ready> ERROR_missing_value;
        std::vector<Ready> ERROR_unknown_value;
        std::vector<Ready> ERROR_assignment_mismatch;
        std::vector<Ready> ERROR_assignment_incomplete;
        std::vector<Ready> ERROR_assignment_wrong_order;
        std::mutex _guard_ERROR;

        out.section
          ( ( boost::format ("Start threads, #compute %1% #load %2%")
            % Parallel_Compute
            % Parallel_Load
            ).str()
          );

        for (std::size_t _ {0}; _ < Parallel_Load; ++_)
        {
          async_load.emplace_back
            ( [&]
              {
                try
                {
                  async_load_running.count_down();

                  while (true)
                  {
                    auto to_prepare (prepare.get());

                    ++number_of_load;

                    memory.at (to_prepare.first) = to_prepare.second;

                    prepared.put (to_prepare.second);
                  }
                }
                catch (QPrepare::interrupted)
                {
                  return;
                }
              }
            );
        }

        for (std::size_t _ {0}; _ < Parallel_Compute; ++_)
        {
          async_compute.emplace_back
            ( [&]
              {
                try
                {
                  async_compute_running.count_down();

                  while (true)
                  {
                    auto to_compute (ready.get());

                    auto const stencil (to_compute.first);
                    auto const assignment (to_compute.second);

                    computations.count_down();

                    {
                      std::lock_guard<std::mutex> const _ {_guard_computed};

                      if (!computed.emplace (stencil).second)
                      {
                        std::lock_guard<std::mutex> const _ {_guard_ERROR};

                        ERROR_duplicated_computation.emplace_back (to_compute);
                      }
                    }

                    std::unordered_set<Coordinate> in_memory;

                    for (auto slot_and_coordinate : assignment)
                    {
                      auto const slot (slot_and_coordinate.first);
                      auto const value (memory.at (slot));

                      in_memory.emplace (value);

                      if (value != slot_and_coordinate.second)
                      {
                        std::lock_guard<std::mutex> const _ {_guard_ERROR};

                        ERROR_assignment_mismatch.emplace_back (to_compute);
                      }

                      used.put (value);
                    }

                    auto const point (to_point (Y, stencil));
                    auto assigned (assignment.cbegin());

                    for (auto value : neighbors (X, Y, R, point))
                    {
                      if (assigned == assignment.end())
                      {
                        std::lock_guard<std::mutex> const _ {_guard_ERROR};

                        ERROR_assignment_incomplete.emplace_back (to_compute);
                      }
                      else if (assigned->second != value)
                      {
                        std::lock_guard<std::mutex> const _ {_guard_ERROR};

                        ERROR_assignment_wrong_order.emplace_back (to_compute);
                      }
                      else if (in_memory.count (value) != 1)
                      {
                        std::lock_guard<std::mutex> const _ {_guard_ERROR};

                        ERROR_missing_value.emplace_back (to_compute);
                      }
                      else
                      {
                        ++assigned;
                        in_memory.erase (value);
                      }
                    }

                    if (!in_memory.empty())
                    {
                      std::lock_guard<std::mutex> const _ {_guard_ERROR};

                      ERROR_unknown_value.emplace_back (to_compute);
                    }
                  }
                }
                catch (QReady::interrupted)
                {
                  return;
                }
              }
            );
        }

        std::thread async_used
          { [&]
            {
              try
              {
                async_used_running.count_down();

                while (true)
                {
                  cache.free (used.get());
                }
              }
              catch (QUsed::interrupted)
              {
                return;
              }
            }
          };

        std::thread async_prepared
          { [&]
            {
              try
              {
                async_prepared_running.count_down();

                while (true)
                {
                  cache.prepared (prepared.get());
                }
              }
              catch (QPrepared::interrupted)
              {
                return;
              }
            }
          };

        async_load_running.wait();
        async_compute_running.wait();
        async_used_running.wait();
        async_prepared_running.wait();

        out.section ("Execute");

        auto alloc
          ( [&] (Point point)
            {
              auto const stencil (to_coordinate (Y, point));

              cache.alloc (stencil, neighbors (X, Y, R, point));
            }
          );

        auto const H (2 * (R - 1) + 1);
        Coordinate const B ((Y * H + M) / M);
        auto const YB (Y/B);

        for (Coordinate b {0}; b < B; ++b)
        for (Coordinate x {0}; x < X; ++x)
        {
          for (Coordinate y {0}; y < YB; ++y)
          {
            alloc (Point {x, b * YB + y});
          }
        }
        for (Coordinate x {0}; x < X; ++x)
        {
          for (Coordinate y {B*YB}; y < Y; ++y)
          {
            alloc (Point {x, y});
          }
        }

        computations.wait();

        out << parameter
            << " number of load " << number_of_load
            << " virtual number of load " << sum_neighbors
            << std::endl
          ;

        prepare.interrupt();
        ready.interrupt();
        used.interrupt();
        prepared.interrupt();

        for (auto& _ : async_compute)
        {
          _.join();
        }
        for (auto& _ : async_load)
        {
          _.join();
        }
        async_used.join();
        async_prepared.join();

        BOOST_REQUIRE (ERROR_duplicated_computation.empty());
        BOOST_REQUIRE (ERROR_missing_value.empty());
        BOOST_REQUIRE (ERROR_unknown_value.empty());
        BOOST_REQUIRE (ERROR_assignment_mismatch.empty());
        BOOST_REQUIRE (ERROR_assignment_incomplete.empty());
        BOOST_REQUIRE (ERROR_assignment_wrong_order.empty());
      }
    }
  }
}
