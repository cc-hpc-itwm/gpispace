// control loop with multi-token condition, mirko.rahn@itwm.fraunhofer.de

#include <we/net.hpp>
#include <we/function/trans.hpp>
#include <we/function/cond.hpp>

#include "timer.hpp"

#include <string>

#include <boost/function.hpp>

using std::cout;
using std::endl;

typedef unsigned long token_t;
typedef std::string place_t;
typedef std::string transition_t;
typedef unsigned short edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;

static token_t max (100000);

typedef boost::unordered_map<petri_net::pid_t,token_t> map_t;
typedef Function::Transition::Traits<token_t>::token_on_place_t top_t;

static unsigned long cnt_trans (0);

static pnet_t::output_t trans_step 
( const petri_net::pid_t pid_state
, const petri_net::pid_t pid_increment
, const pnet_t::input_t & input
, const pnet_t::output_descr_t & output_descr
)
{
  ++cnt_trans;

  pnet_t::output_t output;

  map_t m;

  for ( pnet_t::input_t::const_iterator it (input.begin())
      ; it != input.end()
      ; ++it
      )
    m[Function::Transition::get_pid<token_t>(*it)]
      = Function::Transition::get_token<token_t>(*it);

  output.push_back (top_t (m[pid_state] + m[pid_increment], pid_state));
  output.push_back (top_t (m[pid_increment], pid_increment));

  return output;
}

static unsigned long cnt_cond_step (0);

static bool cond_step 
( const petri_net::pid_t pid_state
, pnet_t::choices_t & choices
)
{
  ++cnt_cond_step;

  for ( ; choices.has_more(); ++choices)
    {
      map_t m;

      pnet_t::choice_star_it choice (*choices);

      for ( ; choice.has_more(); ++choice)
        m[choice->first] = choice->second.first;

      if (m[pid_state] < max)
        return true;
    }

  return false;
}

static unsigned long cnt_cond_break (0);

static bool cond_break
( const petri_net::pid_t pid_state
, pnet_t::choices_t & choices
)
{
  ++cnt_cond_break;

  for ( ; choices.has_more(); ++choices)
    {
      map_t m;

      pnet_t::choice_star_it choice (*choices);

      for ( ; choice.has_more(); ++choice)
        m[choice->first] = choice->second.first;

      if (!(m[pid_state] < max))
        return true;
    }

  return false;
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
  petri_net::pid_t pid_increment (net.add_place ("increment"));

  petri_net::pid_t tid_step (net.add_transition ("step"));
  petri_net::pid_t tid_break (net.add_transition ("break"));

  edge_cnt_t e (0);

  net.add_edge (edge_t (e++, "get state"), connection_t (PT, tid_step, pid_state));
  net.add_edge (edge_t (e++, "set state"), connection_t (TP, tid_step, pid_state));
  net.add_edge (edge_t (e++, "get increment"), connection_t (PT, tid_step, pid_increment));
  net.add_edge (edge_t (e++, "set set increment"), connection_t (TP, tid_step, pid_increment));

  net.add_edge (edge_t (e++, "get state"), connection_t (PT, tid_break, pid_state));

  net.set_transition_function 
    ( tid_step
    , Function::Transition::Generic<token_t>
      ( boost::bind ( &trans_step
                    , pid_state
                    , pid_increment
                    , _1
                    , _2
                    )
      )
    );

  net.set_choice_condition_function 
    ( tid_step
    , Function::Condition::Choice::Generic<token_t>
      ( boost::bind ( &cond_step
                    , pid_state
                    , _1
                    )
      )
    );

  net.set_choice_condition_function 
    ( tid_break
    , Function::Condition::Choice::Generic<token_t>
      ( boost::bind ( &cond_break
                    , pid_state
                    , _1
                    )
      )
    );

  net.put_token (pid_state, 0);
  net.put_token (pid_increment, 1);

  marking (net);

  unsigned long f (0);

  {
    Timer_t timer ("fire", max + 1);

    while (!net.enabled_transitions().empty())
      {
        net.fire(net.enabled_transitions().at(0));
        ++f;
      }
  }

  cout << "fire = " << f << endl;
  cout << "cnt_trans = " << cnt_trans << endl;
  cout << "cnt_cond_step = " << cnt_cond_step << endl;
  cout << "cnt_cond_break = " << cnt_cond_break << endl;

  marking (net);

  return EXIT_SUCCESS;
}
