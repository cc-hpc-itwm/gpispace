// demonstrate timed petri nets, mirko.rahn@itwm.fraunhofer.de

#include <net.hpp>

#include <iostream>
#include <iomanip>

#include <tr1/unordered_map>
#include <tr1/random>

#include <boost/function.hpp>

typedef unsigned int token_t;
typedef std::string place_t;
typedef std::string transition_t;
typedef unsigned short edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

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
    }
}

static void trans_work ( const pid_collection_t & pid
                       , map_t & m
                       , pnet_t::output_t & output
                       )
{
  output.push_back (top_t (m[pid.pre_work], pid.post_work));
}

static void trans_finish ( const pid_collection_t & pid
                         , map_t & m
                         , pnet_t::output_t & output
                         )
{
  output.push_back (top_t (m[pid.max], pid.max));
  output.push_back (top_t (token_t(), pid.queue));

  if (m[pid.post_work] >= m[pid.max] - 1)
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

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "[" << n.place (*p) << ":";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << " " << *tp;

      cout << "]";
    }
  cout << endl;
}

template<typename Engine>
static petri_net::tid_t fire_random_transition (pnet_t & n, Engine & engine)
{
  pnet_t::enabled_t t (n.enabled_transitions());

  std::tr1::uniform_int<pnet_t::enabled_t::size_type> uniform (0,t.size()-1);

  return n.fire (t.at(uniform (engine)));
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

  net.put_token (pid.max, 100);
  net.put_token (pid.i, 0);

  set_trans (net, tid_gen, pid, trans_gen);
  set_trans (net, tid_work, pid, trans_work);
  set_trans (net, tid_finish, pid, trans_finish);
  set_trans (net, tid_finalize, pid, trans_finalize);

  for (int i(0); i < 3; ++i)
    net.put_token (pid.queue);

  marking (net);

  std::tr1::mt19937 engine;

  while (!net.enabled_transitions().empty())
    {
      const petri_net::tid_t tid (fire_random_transition(net, engine));

      cout << std::setw(10) << net.transition (tid) << " # ";

      marking (net);
    }
}
