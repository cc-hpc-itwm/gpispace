// demonstrate timed petri nets, mirko.rahn@itwm.fraunhofer.de
// use separate extractor and injector

#include <we/net.hpp>
#include <we/concurrent/deque.hpp>
#include "timer.hpp"

#include <pthread.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <deque>

#include <boost/unordered_map.hpp>

#include <boost/random.hpp>

#include <boost/function.hpp>

#include <boost/thread.hpp>

typedef unsigned int token_t;
typedef std::string place_t;
struct transition_t
{
public:
  std::string t;

  transition_t () : t("transition without a name") {}
  transition_t (const std::string & _t) : t(_t) {}

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP(t);
  }

  template<typename T>
  bool condition (T &) const { return true; }
};

inline std::size_t hash_value (const transition_t & t)
{
  boost::hash<std::string> h;

  return h (t.t);
}

inline std::size_t operator == (const transition_t & x, const transition_t & y)
{
  return x.t == y.t;
}
typedef unsigned short edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

// ************************************************************************* //

#ifndef _NUM_WORKER
static const unsigned int NUM_WORKER (10);
#else
static const unsigned int NUM_WORKER (_NUM_WORKER);
#endif

#ifndef _NUM_INJECTOR
static const unsigned int NUM_INJECTOR (2);
#else
static const unsigned int NUM_INJECTOR (_NUM_INJECTOR);
#endif

#ifndef _NUM_PACKET
static const token_t NUM_PACKET (100);
#else
static const token_t NUM_PACKET (_NUM_PACKET);
#endif

#ifndef _QUEUE_DEPTH_IN_NET
static const unsigned int QUEUE_DEPTH_IN_NET (2 * NUM_WORKER);
#else
static const unsigned int QUEUE_DEPTH_IN_NET (_QUEUE_DEPTH_IN_NET);
#endif

#ifndef _QUEUE_DEPTH_FOR_WORK_QUEUE
static const unsigned int QUEUE_DEPTH_FOR_WORK_QUEUE (NUM_WORKER);
#else
static const unsigned int QUEUE_DEPTH_FOR_WORK_QUEUE (_QUEUE_DEPTH_FOR_WORK_QUEUE);
#endif

#ifndef _QUEUE_DEPTH_FOR_RESULT_QUEUE
static const unsigned int QUEUE_DEPTH_FOR_RESULT_QUEUE (NUM_WORKER);
#else
static const unsigned int QUEUE_DEPTH_FOR_RESULT_QUEUE (_QUEUE_DEPTH_FOR_RESULT_QUEUE);
#endif

#ifndef _MEAN
static const double mean (1.0);
#else
static const double mean (_MEAN);
#endif

#ifndef _SIGMA
static const double sigma (mean/5.0);
#else
static const double sigma (_SIGMA);
#endif

#ifndef _LOG_LEVEL
static const int log_level (2);
#else
static const int log_level (_LOG_LEVEL);
#endif

// ************************************************************************* //
// some boilerplate

struct edge
{
private:
  edge_cnt_t e;
public:
  edge () : e(0) {}
  edge_t make (const std::string & name) { return edge_t (e++, name); }
};

struct pid_collection_t
{
public:
  petri_net::pid_t max;
  petri_net::pid_t i;
  petri_net::pid_t queue;
  petri_net::pid_t pre_work;
  petri_net::pid_t post_work;
  petri_net::pid_t done_work;
  petri_net::pid_t done_gen;
  petri_net::pid_t all_done;
};

typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;
typedef boost::unordered_map<petri_net::pid_t,token_t> map_t;
typedef boost::function<void ( const pid_collection_t &
                             , map_t & m
                             , pnet_t::output_t &
                             )
                       > trans_t;
typedef Function::Transition::Traits<token_t>::token_on_place_t top_t;

static map_t build_map (const pnet_t::input_t & input)
{
  map_t m;

  for ( pnet_t::input_t::const_iterator it (input.begin())
      ; it != input.end()
      ; ++it
      )
    m[Function::Transition::get_pid<token_t>(*it)]
      = Function::Transition::get_token<token_t>(*it);

  return m;
}

static pnet_t::output_t make_trans ( const pid_collection_t & pid
                                   , const trans_t & f
                                   , const pnet_t::input_t & input
                                   , const pnet_t::output_descr_t &
                                   )
{
  map_t m (build_map (input));

  pnet_t::output_t output;

  f (pid, m, output);

  return output;
}

static void set_trans ( pnet_t & net
                      , const petri_net::tid_t & tid
                      , const pid_collection_t & pid
                      , const trans_t & f
                      )
{
  net.set_transition_function (tid, Function::Transition::Generic<token_t>
                                    (boost::bind(&make_trans, pid, f, _1, _2))
                              );
}

// ************************************************************************* //
// the real transition functions, stripped down to the bare minimum

static void trans_gen ( const pid_collection_t & pid
                      , map_t & m
                      , pnet_t::output_t & output
                      )
{
  output.push_back (top_t (m[pid.max], pid.max));

  if (m[pid.i] < m[pid.max])
    {
      output.push_back (top_t (m[pid.i] + 1, pid.i));
      output.push_back (top_t (m[pid.i], pid.pre_work));
    }
  else
    {
      output.push_back (top_t (m[pid.i], pid.done_gen));
      output.push_back (top_t (token_t(), pid.queue));
    }
}

// thread safe random number generation, unclear whether or not neccessary
template<typename Engine>
struct random_usec
{
private:
  typedef boost::normal_distribution<double> dist_t;
  boost::variate_generator<Engine, dist_t> rand;
  boost::mutex mutex;

  const double left;
  const double right;

public:
  random_usec (const Engine & engine)
    : rand (engine, dist_t (mean, sigma))
    , left (mean-sigma)
    , right (mean+sigma)
  {}
  unsigned int usec (void)
  {
    boost::lock_guard<boost::mutex> lock (mutex);

    return static_cast<unsigned int>
      (1e6 * std::min (std::max (left, rand()), right));
  }
};

template<typename Engine>
static void trans_work ( random_usec<Engine> & random
                       , const pid_collection_t & pid
                       , map_t & m
                       , pnet_t::output_t & output
                       )
{
  unsigned int usec (random.usec());

  while (usec > 0)
    {
      const unsigned int s (std::min (999999u, usec));

      usleep (s);

      usec -= s;
    }

  output.push_back (top_t (m[pid.pre_work], pid.post_work));
}

static void trans_finish ( const pid_collection_t & pid
                         , map_t & m
                         , pnet_t::output_t & output
                         )
{
  output.push_back (top_t (m[pid.max], pid.max));
  output.push_back (top_t (token_t(), pid.queue));

  if (m[pid.post_work] + 1 >= m[pid.max])
    output.push_back (top_t (m[pid.post_work] + 1, pid.done_work));
}

static void trans_finalize ( const pid_collection_t & pid
                           , map_t & m
                           , pnet_t::output_t & output
                           )
{
  if (m[pid.done_gen] != m[pid.done_work])
    throw std::runtime_error ("STRANGE! done_gen != done_work!");

  output.push_back (top_t (m[pid.done_gen], pid.all_done));
}

// ************************************************************************* //
// output marking and fire

using std::cout;
using std::endl;

static std::ostream & operator << (std::ostream & s, const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      s << "[" << n.get_place (*p) << ":";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        s << " " << *tp;

      s << "]";
    }

  return s;
}

// ************************************************************************* //
// div log stuff

struct show_token_input_t
{
  show_token_input_t ( const pnet_t & n
                     , const pnet_t::token_input_t & inp
                     )
    : net (n)
    , input (inp)
  {}

  const pnet_t & net;
  const pnet_t::token_input_t & input;
};

static std::ostream & operator << ( std::ostream & s
                                  , const show_token_input_t & show_token_input
                                  )
{
  return s << "{"
           << Function::Transition::get_token<token_t>(show_token_input.input)
           << " on "
           << show_token_input.net.get_place (Function::Transition::get_pid<token_t>(show_token_input.input))
           << "}";
}

struct show_activity_t
{
  show_activity_t ( const pnet_t & n
                  , const pnet_t::activity_t & a
                  )
    : net (n)
    , activity (a)
  {}

  const pnet_t & net;
  const pnet_t::activity_t & activity;
};

static std::ostream & operator << ( std::ostream & s
                                  , const show_activity_t & show_activity
                                  )
{
  const pnet_t & net (show_activity.net);
  const pnet_t::activity_t & activity (show_activity.activity);

  s << "activity" << ": " << net.get_transition (activity.tid).t << ":";

  s << " input: ";

  for ( pnet_t::input_t::const_iterator it (activity.input.begin())
      ; it != activity.input.end()
      ; ++it
      )
    s << show_token_input_t (net, *it);

  return s;
}

struct show_output_t
{
  show_output_t ( const pnet_t & n
                , const pnet_t::output_t & o
                )
    : net (n)
    , output (o)
  {}

  const pnet_t & net;
  const pnet_t::output_t & output;
};

static std::ostream & operator << ( std::ostream & s
                                  , const show_output_t & show_output
                                  )
{
  const pnet_t & net (show_output.net);
  const pnet_t::output_t & output (show_output.output);

  s << " output: ";

  for ( pnet_t::output_t::const_iterator it (output.begin())
      ; it != output.end()
      ; ++it
      )
    s << "{"
      << Function::Transition::get_token<token_t>(*it)
      << " on "
      << net.get_place (Function::Transition::get_pid<token_t>(*it))
      << "}";

  return s;
}

// ************************************************************************* //

static unsigned int worker_id (0);
static boost::mutex mutex_worker_id;
static unsigned int get_worker_id (void)
{
  boost::lock_guard<boost::mutex> lock (mutex_worker_id);

  unsigned int i (worker_id++);

  return i;
}

static unsigned int injector_id (0);
static boost::mutex mutex_injector_id;
static unsigned int get_injector_id (void)
{
  boost::lock_guard<boost::mutex> lock (mutex_injector_id);

  unsigned int i (injector_id++);

  return i;
}

static boost::mutex mutex_out;
static void do_log (const std::string & msg, const int & level)
{
  if (level < log_level)
    {
      boost::lock_guard<boost::mutex> lock (mutex_out);

      cout << msg << endl;
         cout << std::flush;
    }
}

#define LOG(lvl,msg) {std::ostringstream s; s << msg; do_log(s.str(),lvl);}
#define HEAD(h) h << "." << descr->iThread << ": "
#define WLOG(lvl,msg) LOG(lvl,HEAD("worker") << msg)
#define ELOG(lvl,msg) LOG(lvl,HEAD("extract") << msg)
#define ILOG(lvl,msg) LOG(lvl,HEAD("inject") << msg)

// ************************************************************************* //

// sequentialize critical accesses to the net
template<typename NET>
struct shared_net
{
public:
  typedef unsigned long cnt_t;

private:
  typedef typename NET::activity_t activity_t;
  typedef typename NET::output_t output_t;

  NET & n;
  cnt_t _extract;
  cnt_t _inject;
  boost::mutex mutex;
  boost::condition_variable cond_inject;
  boost::mt19937 engine;

public:
  shared_net (NET & _net) : n (_net), _extract(0), _inject(0) {}

  const NET & net (void) const { return n; }

  void wait_inject (void)
  {
    boost::unique_lock<boost::mutex> lock (mutex);

    const cnt_t i (_inject);

    while (n.enabled_transitions().empty() && i == _inject)
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

    ++_extract;

    return n.extract_activity_random (engine);
  }

  bool done (void)
  {
    boost::lock_guard<boost::mutex> lock (mutex);

    return (_extract == _inject && n.enabled_transitions().empty());
  }

  bool has_enabled (void)
  {
    boost::lock_guard<boost::mutex> lock (mutex);

    return (!n.enabled_transitions().empty());
  }
};

typedef concurrent::deque<pnet_t::activity_t> deque_activity_t;
typedef concurrent::deque<pnet_t::output_t> deque_output_t;

template<typename NET>
struct param_t
{
  shared_net<NET> net;
  deque_activity_t activity;
  deque_output_t output;

  param_t ( NET & _net
          , deque_activity_t::size_type max_activity
          , deque_output_t::size_type max_output
          )
    : net (_net)
    , activity (max_activity)
    , output (max_output)
  {}
};

struct descr_t
{
public:
  const unsigned int iThread;
  double time;
  unsigned long cnt_clock;
  descr_t (const unsigned int & _iThread)
    : iThread (_iThread), time (0.0), cnt_clock (0)
  {}
  void start_clock (void) { ++cnt_clock; time -= statistic::current_time(); }
  void stop_clock (void) { time += statistic::current_time(); }
  std::string info (void)
  {
    std::ostringstream s;

    s << "time " << time << " cnt_clock " << cnt_clock;

    return s.str();
  }
};

static boost::mutex mutex_max_worker_time;
double max_worker_time (0.0);

static inline void exit_handler_show_time_w (void * arg)
{
  descr_t * descr ((descr_t *)arg);

  WLOG (0, descr->info());

  boost::lock_guard<boost::mutex> lock (mutex_max_worker_time);

  max_worker_time = std::max (max_worker_time, descr->time);

  delete descr;
}

static inline void exit_handler_show_time_i (void * arg)
{
  descr_t * descr ((descr_t *)arg);

  ILOG (0, descr->info());

  delete descr;
}

// as many as you like
//static void * worker (void *) __attribute__((noreturn));
template<typename NET>
static void * worker (void * arg)
{
  param_t<NET> * p ((param_t<NET> *)arg);
  descr_t * descr (new descr_t (get_worker_id()));

  WLOG (0, "START");

  pthread_cleanup_push (exit_handler_show_time_w, descr);

  while (1)
    {
      WLOG (1, "GET");

      const pnet_t::activity_t activity (p->activity.get());

      WLOG (1, "RUN " << show_activity_t (p->net.net(), activity));

      descr->start_clock();

      const pnet_t::output_t output (p->net.net().run_activity (activity));

      descr->stop_clock();

      WLOG (1, "PUT" << show_output_t (p->net.net(), output));

      p->output.put (output);
    }

  pthread_cleanup_pop (true);

  return NULL;
}

// as many as you want
template<typename NET>
static void * injector (void * arg)
{
  param_t<NET> * p ((param_t<NET> *) arg);
  descr_t * descr (new descr_t (get_injector_id()));

  ILOG (0, "START");

  pthread_cleanup_push (exit_handler_show_time_i, descr);

  while (1)
    {
      const pnet_t::output_t output (p->output.get());

      ILOG (1, "GET " << show_output_t (p->net.net(), output));

      descr->start_clock();

      p->net.inject (output);

      descr->stop_clock();

      ILOG (1, "INJECT " << show_output_t (p->net.net(), output));
    }

  pthread_cleanup_pop (true);

  return NULL;
}

// one only!
// harder to make it work with more than one extractors, since the
// decision to extract could be badly influenced by other extractions
// however, there is no reason to work on a single net with more than
// one injector/extractor!?
template<typename NET>
static void * extractor (void * arg)
{
  param_t<NET> * p ((param_t<NET> *) arg);
  descr_t * descr (new descr_t (0));

  ELOG (0, "START");

  do
    {
      while (p->net.has_enabled()) // here: problem when more than one extractor
        {
          const pnet_t::activity_t activity (p->net.extract());

          ELOG (1, "EXTRACT " << show_activity_t (p->net.net(), activity));

          descr->start_clock();

          p->activity.put (activity);

          descr->stop_clock();

          ELOG (1, "PUT " << show_activity_t (p->net.net(), activity));
        }

      p->net.wait_inject();
      // ^comment this to get a busy waiting extractor
    }
  while (!p->net.done());

  ELOG (0, "DONE");

  ELOG (0, descr->info());

  delete descr;

  return NULL;
}

// ************************************************************************* //

using petri_net::connection_t;
using petri_net::PT;
using petri_net::TP;

int
main ()
{
  Timer_t timer ("NUM_WORKER", NUM_WORKER);
  pnet_t net;
  pid_collection_t pid;

  pid.max = net.add_place ("max");
  pid.i = net.add_place ("i");
  pid.queue = net.add_place ("queue");
  pid.pre_work = net.add_place ("pre_work");
  pid.post_work = net.add_place ("post_work");
  pid.done_work = net.add_place ("done_work");
  pid.done_gen = net.add_place ("done_gen");
  pid.all_done = net.add_place ("all_done");

  petri_net::tid_t tid_gen (net.add_transition (transition_t("generate")));
  petri_net::tid_t tid_work (net.add_transition (transition_t("work")));
  petri_net::tid_t tid_finish (net.add_transition (transition_t ("finish")));
  petri_net::tid_t tid_finalize (net.add_transition (transition_t ("finalize")));

  edge e;

  net.add_edge (e.make("m"), connection_t (PT, tid_gen, pid.max));
  net.add_edge (e.make("m"), connection_t (TP, tid_gen, pid.max));
  net.add_edge (e.make("i"), connection_t (PT, tid_gen, pid.i));
  net.add_edge (e.make("i"), connection_t (TP, tid_gen, pid.i));
  net.add_edge (e.make("q"), connection_t (PT, tid_gen, pid.queue));
  net.add_edge (e.make("w"), connection_t (TP, tid_gen, pid.pre_work));
  net.add_edge (e.make("d"), connection_t (TP, tid_gen, pid.done_gen));

  net.add_edge (e.make("i"), connection_t (PT, tid_work, pid.pre_work));
  net.add_edge (e.make("i"), connection_t (TP, tid_work, pid.post_work));

  net.add_edge (e.make("i"), connection_t (PT, tid_finish, pid.post_work));
  net.add_edge (e.make("i"), connection_t (TP, tid_finish, pid.done_work));
  net.add_edge (e.make("m"), connection_t (PT, tid_finish, pid.max));
  net.add_edge (e.make("q"), connection_t (TP, tid_finish, pid.queue));

  net.add_edge (e.make("w"), connection_t (PT, tid_finalize, pid.done_work));
  net.add_edge (e.make("g"), connection_t (PT, tid_finalize, pid.done_gen));
  net.add_edge (e.make("f"), connection_t (TP, tid_finalize, pid.all_done));

  net.put_token (pid.max, NUM_PACKET);
  net.put_token (pid.i, 0);

  set_trans (net, tid_gen, pid, trans_gen);
  set_trans (net, tid_finish, pid, trans_finish);
  set_trans (net, tid_finalize, pid, trans_finalize);

  boost::mt19937 engine;
  random_usec<boost::mt19937> random_usec(engine);

  set_trans ( net
            , tid_work
            , pid
            , boost::bind ( &trans_work<boost::mt19937>
                          , boost::ref(random_usec)
                          , _1
                          , _2
                          , _3
                          )
            );

  for (unsigned int i(0); i < QUEUE_DEPTH_IN_NET; ++i)
    net.put_token (pid.queue);

  cout << net << endl;

  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  pthread_t t_extractor;
  pthread_t t_injector[NUM_INJECTOR];
  pthread_t t_worker[NUM_WORKER];

  param_t<pnet_t> p ( net
                    , QUEUE_DEPTH_FOR_WORK_QUEUE
                    , QUEUE_DEPTH_FOR_RESULT_QUEUE
                    );

  pthread_create(&t_extractor, &attr, extractor<pnet_t>, &p);

  for (unsigned int i(0); i < NUM_INJECTOR; ++i)
    pthread_create(t_injector + i, &attr, injector<pnet_t>, &p);

  for (unsigned int w(0); w < NUM_WORKER; ++w)
    pthread_create(t_worker + w, &attr, worker<pnet_t>, &p);


  pthread_join (t_extractor, NULL);


  for (unsigned int i(0); i < NUM_INJECTOR; ++i)
    pthread_cancel (t_injector[i]);

  for (unsigned int w(0); w < NUM_WORKER; ++w)
    pthread_cancel (t_worker[w]);

  for (unsigned int i(0); i < NUM_INJECTOR; ++i)
    pthread_join (t_injector[i], NULL);

  for (unsigned int w(0); w < NUM_WORKER; ++w)
    pthread_join (t_worker[w], NULL);

  cout << net << endl;

  cout << "max_worker_time " << max_worker_time << " ";

  return EXIT_SUCCESS;
}
