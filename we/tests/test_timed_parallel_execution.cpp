// demonstrate timed petri nets, mirko.rahn@itwm.fraunhofer.de

#include <we/net_with_transition_function.hpp>

#include <pthread.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <deque>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

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

static const unsigned int NUM_WORKER (5);
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

typedef petri_net::net_with_transition_function< place_t
                                               , transition_t
                                               , edge_t
                                               , token_t
                                               > pnet_t;
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
public:
  random_usec ( const Engine & engine
              , const double & mean = 1.0
              , const double & sigma = 0.2
              )
    : rand (engine, dist_t (mean, sigma)) {}
  unsigned int usec (void)
  {
    boost::lock_guard<boost::mutex> lock (mutex);

    return static_cast<unsigned int> (std::max (0.0, 1e6 * rand()));
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
// thread safe deque

// DEADLOCK! Attention: A queue wich blocks on a certain maximum
// capacity would lead to a deadlock: The manager puts into the work
// queue, which is full and blocks. At the same time, each worker puts
// into the result queue, which is full as well and blocks.

template<typename T>
struct deque
{
private:
  std::deque<T> q;
  boost::condition_variable cond_put;
  boost::condition_variable cond_get;
  mutable boost::mutex mutex;
public:
  void put (const T & x)
  {
    {
      boost::lock_guard<boost::mutex> lock (mutex);

      q.push_back (x);
    }

    cond_put.notify_one();
  }

  void put (const T & x, const typename std::deque<T>::size_type & watermark)
  {
    boost::unique_lock<boost::mutex> lock (mutex);

    while (q.size() >= watermark)
      cond_get.wait (lock);

    q.push_back (x);

    cond_put.notify_one();
  }

  T get (void)
  {
    boost::unique_lock<boost::mutex> lock (mutex);

    while (q.empty())
      cond_put.wait (lock);

    const T x (q.front()); q.pop_front();

    cond_get.notify_one();

    return x;
  }

  bool empty (void) const
  {
    boost::lock_guard<boost::mutex> lock (mutex);

    return q.empty();
  }

  typename std::deque<T>::size_type size (void) const
  {
    boost::lock_guard<boost::mutex> lock (mutex);

    return q.size();
  }
};

typedef deque<pnet_t::activity_t> deque_activity_t;
typedef deque<pnet_t::output_t> deque_output_t;

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

  cout << std::flush;
}

#define LOG(msg) {std::ostringstream s; s << msg; do_log(s.str());}
#define HEAD(h) h << "." << iThread << ": "
#define WLOG(msg) LOG(HEAD("worker") << msg)
#define MLOG(msg) LOG(HEAD("master") << msg)

// ************************************************************************* //

struct param_t
{
  typedef boost::unordered_set<petri_net::tid_t> tid_set_t;
  pnet_t & net;
  tid_set_t & tid_to_run_on_master; // Not! const, to be changed on runtime
  deque_activity_t activity;
  deque_output_t output;

  param_t (pnet_t & _net, tid_set_t & _tid_to_run_on_master)
    : net (_net), tid_to_run_on_master (_tid_to_run_on_master) {}
};

// as many as you like
//static void * worker (void *) __attribute__((noreturn));
static void * worker (void * arg)
{
  param_t * p ((param_t *)arg);
  const unsigned int iThread (get_id());

  WLOG ("START");

  while (1)
    {
      WLOG ("GET");

      const pnet_t::activity_t activity (p->activity.get());

      WLOG ("RUN " << show_activity_t (p->net, activity));

      const pnet_t::output_t output (p->net.run_activity (activity));

      WLOG ("PUT" << show_output_t (p->net, output));

      // the worker blocks, if the result queue is full
      p->output.put (output, QUEUE_DEPTH_FOR_RESULT_QUEUE);
    }

  return NULL;
}

// one only!
static void * manager (void * arg)
{
  param_t * p ((param_t *) arg);
  const unsigned int iThread (get_id());

  unsigned long extract (0);
  unsigned long inject (0);

  MLOG ("START");

  boost::mt19937 engine;

  do
    {
      while (!p->output.empty()
            || (extract > inject && p->net.enabled_transitions().empty())
            // ^comment this clause to get a busy waiting manager
            || (p->activity.size() >= QUEUE_DEPTH_FOR_WORK_QUEUE)
            )
        {
          MLOG ("GET");

          const pnet_t::output_t output (p->output.get());

          MLOG ("INJECT" << show_output_t (p->net, output));

          p->net.inject_activity_result (output);

          ++inject;
        }

      // the manager puts at most QUEUE_DEPTH_FOR_WORK_QUEUE items into the
      // work queue, it does not block here
      while (  !p->net.enabled_transitions().empty()
            && (p->activity.size() < QUEUE_DEPTH_FOR_WORK_QUEUE)
            )
        {
          const pnet_t::activity_t activity
            (p->net.extract_activity_random (engine));

          ++extract;

         MLOG ("EXTRACT " << show_activity_t (p->net, activity));

          if (  p->tid_to_run_on_master.find (activity.tid)
             != p->tid_to_run_on_master.end()
             )
            {
              // run transition directly

              MLOG ("RUN " << show_activity_t (p->net, activity));

              const pnet_t::output_t output (p->net.run_activity (activity));

              MLOG ("INJECT" << show_output_t (p->net, output));

              p->net.inject_activity_result (output);

              ++inject;
            }
          else
            {
              // put into the queue for the worker threads

              MLOG ("PUT " << show_activity_t (p->net, activity));

              p->activity.put (activity);
            }
        }

      MLOG ("STATE" << ": extract " << extract << ", inject " << inject);
    }
  while (extract > inject);

  MLOG ("DONE");

  return NULL;
}

// ************************************************************************* //

using petri_net::connection_t;
using petri_net::PT;
using petri_net::TP;

int
main ()
{
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

  net.put_token (pid.max, 100);
  net.put_token (pid.i, 0);

  for (unsigned int i(0); i < QUEUE_DEPTH_IN_NET; ++i)
    net.put_token (pid.queue);

  cout << net << endl;

  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  pthread_t t_manager;
  pthread_t t_worker[NUM_WORKER];

  deque_activity_t deque_activity;
  deque_output_t deque_output;

  param_t::tid_set_t tid_to_run_on_master;

  //  tid_to_run_on_master.insert (tid_gen);
  tid_to_run_on_master.insert (tid_finish);
  tid_to_run_on_master.insert (tid_finalize);
  //  tid_to_run_on_master.insert (tid_work);
  // ^ucomment this to get serial execution on the master

  param_t p (net, tid_to_run_on_master);

  pthread_create(&t_manager, &attr, manager, &p);

  for (unsigned int w(0); w < NUM_WORKER; ++w)
    pthread_create(t_worker + w, &attr, worker, &p);

  pthread_join (t_manager, NULL);

  for (unsigned int w(0); w < NUM_WORKER; ++w)
    pthread_cancel (t_worker[w]);

  for (unsigned int w(0); w < NUM_WORKER; ++w)
    pthread_join (t_worker[w], NULL);

  pthread_attr_destroy(&attr);

  cout << net << endl;

  return EXIT_SUCCESS;
}
