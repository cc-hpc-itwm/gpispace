// demonstrate timed petri nets, mirko.rahn@itwm.fraunhofer.de
// use separate extractor and injector

#include <net.hpp>
#include <timer.hpp>

#include <pthread.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <deque>

#include <tr1/unordered_map>
#include <tr1/random>

#include <boost/function.hpp>
#include <boost/thread.hpp>

typedef unsigned int token_t;
typedef std::string place_t;
typedef std::string transition_t;
typedef unsigned short edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

// ************************************************************************* //

#ifndef _NUM_WORKER
static const unsigned int NUM_WORKER (10);
#else
static const unsigned int NUM_WORKER (_NUM_WORKER);
#endif

#ifndef _NUM_PACKET
static const token_t NUM_PACKET (100);
#else
static const token_t NUM_PACKET (_NUM_PACKET);
#endif

static const unsigned int QUEUE_DEPTH_IN_NET (2 * NUM_WORKER);
static const unsigned int QUEUE_DEPTH_FOR_WORK_QUEUE (NUM_WORKER);
static const unsigned int QUEUE_DEPTH_FOR_RESULT_QUEUE (NUM_WORKER);

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
typedef std::tr1::unordered_map<petri_net::pid_t,token_t> map_t;
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
  typedef std::tr1::normal_distribution<double> dist_t;
  std::tr1::variate_generator<Engine, dist_t> rand;
  boost::mutex mutex;

  const double left;
  const double right;

public:
  random_usec ( const Engine & engine
              , const double & mean = 1.0
              , const double & sigma = 0.2
              )
    : rand (engine, dist_t (mean, sigma))
    , left (mean-sigma)
    , right (mean+sigma)
  {}
  useconds_t usec (void)
  {
    boost::lock_guard<boost::mutex> lock (mutex);

    return 1e6 * std::min (std::max (left, rand()), right);
  }
};

template<typename Engine>
static void trans_work ( random_usec<Engine> & random
                       , const pid_collection_t & pid
                       , map_t & m
                       , pnet_t::output_t & output
                       )
{
  usleep (random.usec());

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
// thread safe deque, with bounded depth

template<typename T>
struct deque
{
public:
  typedef typename std::deque<T>::size_type size_type;

  deque (const size_type & _max) : max (_max) {}

private:
  size_type max;
  std::deque<T> q;
  boost::condition_variable cond_put;
  boost::condition_variable cond_get;
  boost::mutex mutex;
 
public:
  // maybe blocking!
  void put (const T & x)
  {
    boost::unique_lock<boost::mutex> lock (mutex);

    while (q.size() >= max)
      cond_get.wait (lock);
      
    q.push_back (x);

    cond_put.notify_one();
  }

  // maybe blocking!
  T get (void) 
  {
    boost::unique_lock<boost::mutex> lock (mutex);

    while (q.empty())
      cond_put.wait (lock);

    const T x (q.front()); q.pop_front();

    cond_get.notify_one();

    return x;
  }

  bool empty (void) const { return q.empty(); }
  size_type size (void) const { return q.size(); }
};

typedef deque<pnet_t::activity_t> deque_activity_t;
typedef deque<pnet_t::output_t> deque_output_t;

// ************************************************************************* //
// div log stuff

typedef std::pair<const pnet_t &,const pnet_t::token_input_t> show_token_input_t;

static std::ostream & operator << ( std::ostream & s
                                  , const show_token_input_t & show_token_input
                                  )
{
  const pnet_t & net (show_token_input.first);
  const pnet_t::token_input_t token_input (show_token_input.second);

  return s << "{"
           << Function::Transition::get_token<token_t>(token_input)
           << " on "
           << net.get_place (Function::Transition::get_pid<token_t>(token_input))
           << "}";
}

typedef std::pair<const pnet_t &,const pnet_t::activity_t> show_activity_t;

static std::ostream & operator << ( std::ostream & s
                                  , const show_activity_t & show_activity
                                  )
{
  const pnet_t & net (show_activity.first);
  const pnet_t::activity_t activity (show_activity.second);

  s << "activity" << ": " << net.get_transition (activity.tid) << ":";

  s << " input: ";

  for ( pnet_t::input_t::const_iterator it (activity.input.begin())
      ; it != activity.input.end()
      ; ++it
      )
    s << show_token_input_t (net, *it);

  return s;
}

typedef std::pair<const pnet_t &,const pnet_t::output_t> show_output_t;

static std::ostream & operator << ( std::ostream & s
                                  , const show_output_t & show_output
                                  )
{
  const pnet_t & net (show_output.first);
  const pnet_t::output_t output (show_output.second);

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

static unsigned int id (0);
static boost::mutex mutex_id;

static unsigned int get_id (void)
{
  boost::lock_guard<boost::mutex> lock (mutex_id);

  unsigned int i (id++);

  return i;
}

static boost::mutex mutex_out;
static void do_log (const std::string & msg)
{
  boost::lock_guard<boost::mutex> lock (mutex_out);

  cout << msg << endl;

  fflush (stdout);
}

#define LOG(msg) {std::ostringstream s; s << msg; do_log(s.str());}
#define HEAD(h) h << "." << descr->iThread << ": "
#define WLOG(msg) LOG(HEAD("worker") << msg)
#define ELOG(msg) LOG(HEAD("extract") << msg)
#define ILOG(msg) LOG(HEAD("inject") << msg)

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
  std::tr1::mt19937 engine;

public:
  shared_net (NET & _net) : n (_net), _extract(0), _inject(0) {}

  const NET & net (void) const { return n; }

  void wait_inject (void)
  {
    boost::unique_lock<boost::mutex> lock (mutex);

    while (n.enabled_transitions().empty() && _extract > _inject)
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
  descr_t (const unsigned int & _iThread) : iThread (_iThread), time (0.0) {}
  void start_clock (void) { time -= current_time(); }
  void stop_clock (void) { time += current_time(); }
};

static boost::mutex mutex_max_worker_time;
double max_worker_time (0.0);

static inline void exit_handler_show_time (void * arg)
{
  descr_t * descr ((descr_t *)arg);

  WLOG ("time " << descr->time);

  boost::lock_guard<boost::mutex> lock (mutex_max_worker_time);

  max_worker_time = std::max (max_worker_time, descr->time);
}

// as many as you like
//static void * worker (void *) __attribute__((noreturn));
template<typename NET>
static void * worker (void * arg)
{
  param_t<NET> * p ((param_t<NET> *)arg);
  descr_t * descr (new descr_t (get_id()));

  WLOG ("START");

  pthread_cleanup_push (exit_handler_show_time, descr);

  while (1)
    {
      WLOG ("GET");

      const pnet_t::activity_t activity (p->activity.get());

      WLOG ("RUN " << show_activity_t (p->net.net(), activity));

      descr->start_clock();

      const pnet_t::output_t output (p->net.net().run_activity (activity));

      descr->stop_clock();

      WLOG ("PUT" << show_output_t (p->net.net(), output));

      p->output.put (output);
    }

  pthread_cleanup_pop (true);

  return NULL;
}

// one only!
// quite easy to work with more than one injectors: do not check for
// net.done(). Instead directly go to the get and cancel the thread
// from outside.
template<typename NET>
static void * injector (void * arg)
{
  param_t<NET> * p ((param_t<NET> *) arg);
  descr_t * descr (new descr_t (get_id()));

  ILOG ("START");

  do
    {
      do
        {
          const pnet_t::output_t output (p->output.get());

          ILOG ("GET " << show_output_t (p->net.net(), output));

          descr->start_clock();

          p->net.inject (output);

          descr->stop_clock();

          ILOG ("INJECT " << show_output_t (p->net.net(), output));
        }
      while (!p->output.empty()); // here: problem when more than one injector
    }
  while (!p->net.done());

  ILOG ("DONE " << descr->time);

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
  descr_t * descr (new descr_t (get_id()));

  ELOG ("START");

  do
    {
      while (p->net.has_enabled()) // here: problem when more than one extractor
        {
          const pnet_t::activity_t activity (p->net.extract());

          ELOG ("EXTRACT " << show_activity_t (p->net.net(), activity));

          descr->start_clock();

          p->activity.put (activity);

          descr->stop_clock();

          ELOG ("PUT " << show_activity_t (p->net.net(), activity));
        }

      p->net.wait_inject();
      // ^comment this to get a busy waiting extractor
    }
  while (!p->net.done());
  
  ELOG ("DONE " << descr->time);

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

  petri_net::tid_t tid_gen (net.add_transition ("generate"));
  petri_net::tid_t tid_work (net.add_transition ("work"));
  petri_net::tid_t tid_finish (net.add_transition ("finish"));
  petri_net::tid_t tid_finalize (net.add_transition ("finalize"));

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

  std::tr1::mt19937 engine;
  random_usec<std::tr1::mt19937> random_usec(engine);

  set_trans ( net
            , tid_work
            , pid
            , boost::bind ( &trans_work<std::tr1::mt19937>
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
  pthread_t t_injector;
  pthread_t t_worker[NUM_WORKER];

  param_t<pnet_t> p ( net
                    , QUEUE_DEPTH_FOR_WORK_QUEUE
                    , QUEUE_DEPTH_FOR_RESULT_QUEUE
                    );

  pthread_create(&t_extractor, &attr, extractor<pnet_t>, &p);
  pthread_create(&t_injector, &attr, injector<pnet_t>, &p);

  for (unsigned int w(0); w < NUM_WORKER; ++w)
    pthread_create(t_worker + w, &attr, worker<pnet_t>, &p);


  pthread_join (t_extractor, NULL);
  pthread_join (t_injector, NULL);

  for (unsigned int w(0); w < NUM_WORKER; ++w)
    pthread_cancel (t_worker[w]);

  for (unsigned int w(0); w < NUM_WORKER; ++w)
    pthread_join (t_worker[w], NULL);

  cout << net << endl;

  cout << "max_worker_time " << max_worker_time << " ";

  return EXIT_SUCCESS;
}
