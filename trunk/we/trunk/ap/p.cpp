// control loop, mirko.rahn@itwm.fraunhofer.de

#include <net.hpp>
#include <function/trans.hpp>
#include <function/cond.hpp>

#include <util/timer.hpp>

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

static const token_t max (1000000);

static unsigned long cnt_cond_lt (0);

static bool cond_lt ( const petri_net::pid_t & pid_value
                    , const token_t & token
                    , const petri_net::pid_t & pid
                    , const petri_net::eid_t &
                    )
{
  ++cnt_cond_lt;

  return (pid != pid_value) || (token < max);
}

static unsigned long cnt_cond_ge (0);

static bool cond_ge ( const token_t & token
                    , const petri_net::pid_t &
                    , const petri_net::eid_t &
                    )
{
  ++cnt_cond_ge;

  return (token >= max);
}

static unsigned long cnt_trans (0);

static pnet_t::output_t trans ( const petri_net::pid_t & pid_value
                              , const petri_net::pid_t & pid_increment
                              , const pnet_t::input_t & input
                              , const pnet_t::output_descr_t & output_descr
                              )
{
  ++cnt_trans;

  pnet_t::output_t output;

  token_t value (0);
  token_t increment (0);

  for ( pnet_t::input_t::const_iterator it (input.begin())
      ; it != input.end()
      ; ++it
      )
    {
      const petri_net::pid_t pid (Function::Transition::get_pid<token_t> (*it));
      const token_t token (Function::Transition::get_token<token_t> (*it));

      if (pid == pid_value)
        {
          value = token;
        }
      else if (pid == pid_increment)
        {
          increment = token;
        }
      else
        throw std::runtime_error ("STRANGE! Unknown input!");
    }

  typedef Function::Transition::Traits<token_t>::token_on_place_t top_t;

  for ( pnet_t::output_descr_t::const_iterator it (output_descr.begin())
      ; it != output_descr.end()
      ; ++it
      )
    if (it->first == pid_value)
      {
        output.push_back (top_t (value + increment, pid_value));
      }
    else if (it->first == pid_increment)
      {
        output.push_back (top_t (increment, pid_increment));
      }
    else
      throw std::runtime_error ("STRANGE! Unknown output!");

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

  petri_net::pid_t pid_value (net.add_place ("value"));
  petri_net::pid_t pid_increment (net.add_place ("increment"));
  petri_net::pid_t pid_final (net.add_place ("final"));
  petri_net::pid_t tid_step (net.add_transition ("step"));
  petri_net::pid_t tid_break (net.add_transition ("break"));

  edge_cnt_t e (0);

  net.add_edge (edge_t (e++, "v"), connection_t (PT, tid_step, pid_value));
  net.add_edge (edge_t (e++, "v"), connection_t (TP, tid_step, pid_value));

  net.add_edge (edge_t (e++, "i"), connection_t (PT, tid_step, pid_increment));
  net.add_edge (edge_t (e++, "i"), connection_t (TP, tid_step, pid_increment));

  net.add_edge (edge_t (e++, "v"), connection_t (PT, tid_break, pid_value));
  net.add_edge (edge_t (e++, "v"), connection_t (TP, tid_break, pid_final));

  net.set_transition_function ( tid_step
                              , Function::Transition::Generic<token_t>
                                ( boost::bind ( &trans
                                              , pid_value
                                              , pid_increment
                                              , _1
                                              , _2
                                              )
                                )
                              );
  net.set_in_condition_function ( tid_step
                                , Function::Condition::In::Generic<token_t>
                                  ( boost::bind ( &cond_lt
                                                , pid_value
                                                , _1
                                                , _2
                                                , _3
                                                )
                                  )
                                );

  net.set_transition_function ( tid_break
                              , Function::Transition::Pass<token_t>()
                              );
  net.set_in_condition_function ( tid_break
                                , Function::Condition::In::Generic<token_t>
                                  (&cond_ge)
                                );

  net.put_token (pid_value, 0);
  net.put_token (pid_increment, 1);

  marking (net);

  unsigned long f (0);

  {
    Timer_t timer ("fire", max + 1);

    pnet_t::enabled_t t (net.enabled_transitions());

    while (!t.empty())
      {
        net.fire(t.at(0));
        t = net.enabled_transitions();
        ++f;
      }
  }

  cout << "fire = " << f << endl;
  cout << "cnt_trans = " << cnt_trans << endl;
  cout << "cnt_cond_lt = " << cnt_cond_lt << endl;
  cout << "cnt_cond_ge = " << cnt_cond_ge << endl;

  marking (net);

  return EXIT_SUCCESS;
}
