#define BOOST_TEST_MODULE id_generator
#include <boost/test/unit_test.hpp>

#include <sdpa/id_generator.hpp>

#include <fhg/util/boost/test/require_exception.hpp>
#include <fhg/util/random_string.hpp>

#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/random.hpp>
#include <boost/unordered_set.hpp>

#include <stdexcept>

namespace
{
  //! \note that boost::format as well as std::string::operator+ have
  //! problems with the zero
  std::string random_string_without_zero()
  {
    static std::string const zero (1, '\0');

    return fhg::util::random_string_without (zero);
  }
}

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
    boost::unordered_set<std::string> _ids;
  };
}

BOOST_AUTO_TEST_CASE (threaded_unique_set_of_id_throws_on_duplicate)
{
  threaded_unique_set_of_id ids;

  std::string const id (random_string_without_zero());

  BOOST_REQUIRE_EQUAL (id, ids.insert (id));

  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&threaded_unique_set_of_id::insert, &ids, id)
    , (boost::format ("duplicate id '%1%'") % id).str()
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

    sdpa::id_generator id_generator (random_string_without_zero());
    threaded_unique_set_of_id ids;

    while (num_threads --> 0)
    {
      threads.add_thread (new boost::thread ( &insert
                                            , boost::ref (id_generator)
                                            , boost::ref (ids)
                                            , random (engine)
                                            , boost::ref (barrier)
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
