#define BOOST_TEST_MODULE UtilSemaphoreTest
#include <boost/test/unit_test.hpp>

#include <stdlib.h>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <fhg/util/thread/semaphore.hpp>

struct table
{
  static const std::size_t TABLE_SIZE = 5;

  table()
    : m_fork (TABLE_SIZE)
  {
    using namespace fhg::thread;

    for (std::size_t i = 0; i < TABLE_SIZE; ++i)
    {
      m_fork[i] = new semaphore (1);
    }
  }

  ~table()
  {
    using namespace fhg::thread;

    BOOST_FOREACH (semaphore * fork, m_fork)
    {
      if (fork->count() != 1)
      {
        std::cerr << "STRANGE! there is still a philosopher dining!!!" << std::endl;
        throw std::runtime_error ("BUMMER!");
      }
      delete fork; fork = 0;
    }
  }

  void pick_up(const std::size_t left_fork, const std::size_t right_fork)
  {
    std::size_t min_fork = std::min (left_fork, right_fork);
    std::size_t max_fork = std::max (left_fork, right_fork);

    m_fork [min_fork]->P();
    m_fork [max_fork]->P();
  }

  void put_back (const std::size_t left_fork, const std::size_t right_fork)
  {
    std::size_t min_fork = std::min (left_fork, right_fork);
    std::size_t max_fork = std::max (left_fork, right_fork);

    m_fork [max_fork]->V();
    m_fork [min_fork]->V();
  }

private:
  std::vector<fhg::thread::semaphore*> m_fork;
};

struct philosopher
{
  explicit
  philosopher ( const std::size_t rank
              , table & t
              , const std::size_t iterations = 2
              )
    : m_rank (rank)
    , m_table (t)
    , m_iterations (iterations)
  {}

  void run ()
  {
    for ( std::size_t i = 0; i < m_iterations; ++i)
    {
      think ();
      eat ();
      sleep ();
    }
  }

private:
  void think ()
  {
    std::cout << "philosopher " << m_rank << " is thinking..." << std::endl;
    usleep (std::max (5000L, random() % 100000));
  }

  void eat ()
  {
    const std::size_t my_fork [] =
      { m_rank % table::TABLE_SIZE
        , (m_rank+1) % table::TABLE_SIZE
      };

    std::cout << "philosopher " << m_rank << " is getting hungry..." << std::endl;
    m_table.pick_up (my_fork[0], my_fork[1]);

    std::cout << "philosopher " << m_rank << " eats delicious food..." << std::endl;
    usleep (std::max (5000L, random() % 100000));

    m_table.put_back (my_fork[0], my_fork[1]);
  }

  void sleep ()
  {
    std::cout << "philosopher " << m_rank << " is getting veeerrrry tired..." << std::endl;
    usleep (std::max (5000L, random() % 100000));
  }

  std::size_t m_rank;
  table & m_table;
  std::size_t m_iterations;
};

BOOST_AUTO_TEST_CASE ( dining_philosophers )
{
  using namespace fhg::thread;
  table wishing_table;
  std::vector<boost::shared_ptr<philosopher> > nerds;
  for (std::size_t i (0); i < table::TABLE_SIZE; ++i)
  {
    boost::shared_ptr<philosopher> nerd
      (new philosopher(i, wishing_table, 2));
    nerds.push_back (nerd);
  }

  std::vector<boost::shared_ptr<boost::thread> > threads;
  for (std::size_t i (0); i < table::TABLE_SIZE; ++i)
  {
    boost::shared_ptr<boost::thread> thread
      (new boost::thread(boost::bind ( &philosopher::run
                                     , nerds[i]
                                     )
                        )
      );
    threads.push_back (thread);
  }

  for ( std::size_t i (0); i < threads.size(); ++i)
  {
    threads[i]->join ();
  }
}

BOOST_AUTO_TEST_CASE ( enter_critical_section )
{
  fhg::thread::semaphore s (1);
  s.P();
  s.V();

  BOOST_CHECK_EQUAL(std::size_t(1), s.count());
}
