#define BOOST_TEST_MODULE id_generator
#include <boost/test/unit_test.hpp>

#include <sdpa/id_generator.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/random_string.hpp>

#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/random.hpp>

#include <functional>
#include <stdexcept>
#include <unordered_set>

namespace
{
  class threaded_unique_set_of_id
  {
  public:
    std::string insert (std::string const& id)
    {
      boost::mutex::scoped_lock const _ (_mutex_ids);

      if (!_ids.insert (id).second)
      {
        throw std::runtime_error
          ((boost::format ("duplicate id '%1%'") % id).str());
      }

      return id;
    }
  private:
    boost::mutex _mutex_ids;
    std::unordered_set<std::string> _ids;
  };
}

BOOST_AUTO_TEST_CASE (threaded_unique_set_of_id_throws_on_duplicate)
{
  threaded_unique_set_of_id ids;

  std::string const id (fhg::util::testing::random_string_without_zero());

  BOOST_REQUIRE_EQUAL (id, ids.insert (id));

  fhg::util::testing::require_exception
    ( [&id, &ids] { ids.insert (id); }
    , std::runtime_error
        ((boost::format ("duplicate id '%1%'") % id).str())
    );
}

namespace
{
  void insert ( sdpa::id_generator& id_generator
              , threaded_unique_set_of_id& ids
              , std::size_t number
              , boost::barrier& barrier
              )
  {
    barrier.wait();

    while (number --> 0)
    {
      ids.insert (id_generator.next());
    }
  }

  void generator_is_unique_with_n_threads (std::size_t num_threads)
  {
    boost::thread_group threads;
    boost::mt19937 engine;
    boost::barrier barrier (num_threads);

    boost::uniform_int<std::size_t> random (100, 1000);

    sdpa::id_generator id_generator (fhg::util::testing::random_string_without_zero());
    threaded_unique_set_of_id ids;

    while (num_threads --> 0)
    {
      threads.add_thread (new boost::thread ( &insert
                                            , std::ref (id_generator)
                                            , std::ref (ids)
                                            , random (engine)
                                            , std::ref (barrier)
                                            )
                         );
    }

    threads.join_all();
  }
}

BOOST_AUTO_TEST_CASE (generator_is_unique_with_one_thread)
{
  generator_is_unique_with_n_threads (1);
}

BOOST_AUTO_TEST_CASE (generator_is_unique_with_two_threads)
{
  generator_is_unique_with_n_threads (2);
}

BOOST_AUTO_TEST_CASE (generator_is_unique_with_threads_from_3_to_20)
{
  for (std::size_t num_threads (3); num_threads < 21; ++num_threads)
  {
    generator_is_unique_with_n_threads (num_threads);
  }
}
