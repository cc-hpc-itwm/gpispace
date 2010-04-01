// control loop with multi-token condition, mirko.rahn@itwm.fraunhofer.de

#include <we/net.hpp>
#include <we/function/trans.hpp>
#include <we/function/cond_exp.hpp>

#include <we/util/show.hpp>

#include "timer.hpp"

#include <string>

#include <boost/function.hpp>
#include <boost/random.hpp>

using std::cout;
using std::endl;

typedef unsigned long token_t;
typedef std::string place_t;
typedef std::string transition_t;
typedef unsigned short edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;

static token_t max (100000);
static unsigned int capacity_value (3);

typedef boost::unordered_map<petri_net::pid_t,token_t> map_t;
typedef Function::Transition::Traits<token_t>::token_on_place_t top_t;

static pnet_t::output_t trans_step 
( const petri_net::pid_t pid_max
, const petri_net::pid_t pid_state
, const petri_net::pid_t pid_value
, const pnet_t::input_t & input
, const pnet_t::output_descr_t & output_descr
)
{
  pnet_t::output_t output;

  map_t m;

  for ( pnet_t::input_t::const_iterator it (input.begin())
      ; it != input.end()
      ; ++it
      )
    m[Function::Transition::get_pid<token_t>(*it)]
      = Function::Transition::get_token<token_t>(*it);

  output.push_back (top_t (m[pid_max], pid_max));
  output.push_back (top_t (m[pid_state] + 1, pid_state));
  output.push_back (top_t (m[pid_state], pid_value));

  return output;
}

static pnet_t::output_t trans_sum
( const pnet_t::input_t & input
, const pnet_t::output_descr_t & output_descr
)
{
  token_t sum (0);

  for ( pnet_t::input_t::const_iterator it (input.begin())
      ; it != input.end()
      ; ++it
      )
    sum += Function::Transition::get_token<token_t>(*it);

  pnet_t::output_t output;

  for ( pnet_t::output_descr_t::const_iterator it (output_descr.begin())
          ; it != output_descr.end()
          ; ++it
      )
    output.push_back (top_t (sum, Function::Transition::get_pid<token_t>(*it)));

  return output;
}

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "[" << n.get_place (*p) << ":";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << " " << *tp;

      cout << "]";
    }
  cout << endl;
}

using petri_net::connection_t;
using petri_net::PT;
using petri_net::TP;

int
main ()
{
  pnet_t net;

  petri_net::pid_t pid_state (net.add_place ("state"));
  petri_net::pid_t pid_max (net.add_place ("max"));
  petri_net::pid_t pid_value (net.add_place ("value"));
  petri_net::pid_t pid_sum (net.add_place ("sum"));

  petri_net::pid_t tid_step (net.add_transition ("step"));
  petri_net::pid_t tid_sum (net.add_transition ("sum"));

  edge_cnt_t e (0);

  net.add_edge (edge_t (e++, "get state"), connection_t (PT, tid_step, pid_state));
  net.add_edge (edge_t (e++, "set state"), connection_t (TP, tid_step, pid_state));
  net.add_edge (edge_t (e++, "get max"), connection_t (PT, tid_step, pid_max));
  net.add_edge (edge_t (e++, "set max"), connection_t (TP, tid_step, pid_max));
  net.add_edge (edge_t (e++, "set val"), connection_t (TP, tid_step, pid_value));

  net.add_edge (edge_t (e++, "get val"), connection_t (PT, tid_sum, pid_value));
  net.add_edge (edge_t (e++, "get sum"), connection_t (PT, tid_sum, pid_sum));
  net.add_edge (edge_t (e++, "set sum"), connection_t (TP, tid_sum, pid_sum));

  net.set_transition_function 
    ( tid_step
    , Function::Transition::Generic<token_t>
      ( boost::bind ( &trans_step
                    , pid_max
                    , pid_state
                    , pid_value
                    , _1
                    , _2
                    )
      )
    );

  net.set_transition_function 
    ( tid_sum
    , Function::Transition::Generic<token_t> (trans_sum)
    );

  net.set_choice_condition_function 
    ( tid_step
    , Function::Condition::Choice::Expression<token_t>
      ("${" + show (pid_state) + "} < ${" + show (pid_max) + "}")
    );

  net.set_capacity (pid_value, capacity_value);

  net.put_token (pid_state, 0);
  net.put_token (pid_state, 0);
  net.put_token (pid_sum, 0);
  net.put_token (pid_max, max);

  marking (net);

  {
    boost::mt19937 engine;

    Timer_t timer ("fire", max + 1);

    while (!net.enabled_transitions().empty())
      {
        net.fire(net.enabled_transitions().at(0));
        //        marking (net);
      }
  }

  marking (net);

  return EXIT_SUCCESS;
}
