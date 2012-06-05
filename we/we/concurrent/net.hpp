// mirko.rahn@itwm.fraunhofer.de

#ifndef _CONCURRENT_NET_HPP
#define _CONCURRENT_NET_HPP

#include <pthread.h>

#include <we/net.hpp>
#include <we/concurrent/deque.hpp>

#include <boost/thread.hpp>
#include <boost/random.hpp>

#include <iostream>

namespace petri_net
{
  // sequentialize critical accesses to the net
  template<typename NET, typename CNT = unsigned long>
  struct shared_t
  {
  private:
    typedef typename NET::activity_t activity_t;
    typedef typename NET::output_t output_t;

    NET & n;
    CNT _extract;
    CNT _inject;

    boost::mutex mutex;
    boost::condition_variable cond_inject;

    boost::mt19937 engine;

  public:
    shared_t (NET & _net)
      : n (_net)
      , _extract(0)
      , _inject(0)
    {}

    void wait_inject_or_done (void)
    {
      boost::unique_lock<boost::mutex> lock (mutex);

      const CNT i (_inject);

      while (  not n.can_fire()
            && i == _inject
            && _extract != _inject // otherwise: done
            )
        cond_inject.wait (lock);
    }

    void inject (const output_t & output)
    {
      boost::lock_guard<boost::mutex> lock (mutex);

      n.inject_activity_result (output);

      ++_inject;

      cond_inject.notify_one();
    }

    activity_t extract (void)
    {
      boost::lock_guard<boost::mutex> lock (mutex);

      const activity_t act (n.extract_activity_random (engine));

      ++_extract;

      return act;
    }

    bool done (void)
    {
      boost::lock_guard<boost::mutex> lock (mutex);

      return (_extract == _inject && not n.can_fire());
    }

    bool can_fire (void)
    {
      boost::lock_guard<boost::mutex> lock (mutex);

      return n.can_fire();
    }
  };

  namespace thread
  {
    template<typename NET>
    struct param_t
    {
      typedef typename NET::activity_t activity_t;
      typedef typename NET::output_t output_t;

      shared_t<NET> shared;
      concurrent::deque<activity_t> activity;
      concurrent::deque<output_t> output;

      param_t
      ( NET & _net
      , const typename concurrent::deque<activity_t>::size_type & max_activity
      , const typename concurrent::deque<output_t>::size_type & max_output
      )
        : shared (_net)
        , activity (max_activity)
        , output (max_output)
      {}
    };

    // as many as you want
    template<typename NET>
    static void * injector (void * arg)
    {
      param_t<NET> * p ((param_t<NET> *) arg);

      while (1)
        p->shared.inject (p->output.get());

      return NULL;
    }

    // one only!
    template<typename NET>
    static void * extractor (void * arg)
    {
      param_t<NET> * p ((param_t<NET> *) arg);

      do
        {
          while (p->shared.can_fire())
            p->activity.put (p->shared.extract());

          p->shared.wait_inject_or_done ();
          // ^comment this to get a busy waiting extractor
        }
      while (!p->shared.done());

      return NULL;
    }
  }

  template<typename NET>
  class thread_safe_t
  {
  private:
    typedef typename NET::activity_t activity_t;
    typedef typename NET::output_t output_t;

    NET & net;

    thread::param_t<NET> param;

    const unsigned int num_injector;

    pthread_attr_t attr;
    pthread_t extractor;
    pthread_t * injector;

  public:
    thread_safe_t
    ( NET & _net
    , const typename concurrent::deque<activity_t>::size_type & _max_activity = 100
    , const typename concurrent::deque<output_t>::size_type & _max_output = 100
    , const unsigned int _num_injector = 1
    )
      : net (_net)
      , param (_net, _max_activity, _max_output)
      , num_injector (_num_injector)
    {
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

      injector = new pthread_t[num_injector];

      pthread_create (&extractor, &attr, thread::extractor<NET>, &param);

      for (unsigned int i(0); i < num_injector; ++i)
        pthread_create(injector + i, &attr, thread::injector<NET>, &param);
    }

    ~thread_safe_t (void)
    {
      for (unsigned int i(0); i < num_injector; ++i)
        pthread_cancel(injector[i]);

      for (unsigned int i(0); i < num_injector; ++i)
        pthread_join(injector[i], NULL);

      delete[] injector;
    }

    void wait (void) { pthread_join (extractor, NULL); }

    void fire (void)
    {
      const activity_t activity (param.activity.get());
      const output_t output (net.run_activity(activity));
      param.output.put(output);
    }
  };
}

#endif
