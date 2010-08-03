// control loop, mirko.rahn@itwm.fraunhofer.de

#include <we/net.hpp>
#include <we/function/trans.hpp>
#include <we/function/cond.hpp>

#include <fhg/util/show.hpp>

#include "timer.hpp"

#include <string>

#include <boost/function.hpp>

using std::cout;
using std::endl;

typedef long token_t;
typedef std::string place_t;

typedef unsigned short edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

struct transition_t
{
public:
  std::string t;
  mutable Function::Condition::Expression<token_t> cond;

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP(t);
    ar & BOOST_SERIALIZATION_NVP(cond);
  }

  transition_t ( const std::string & _t
               , const std::string & _cond
               ) : t (_t), cond (_cond) {}

  bool condition (Function::Condition::Traits<token_t>::choices_t & choices)
    const
  {
    return cond (choices);
  }
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

typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;

static const token_t max (100000);

typedef boost::unordered_map<petri_net::pid_t,token_t> map_t;
typedef Function::Transition::Traits<token_t>::token_on_place_t top_t;


static pnet_t::output_t trans ( const petri_net::pid_t & pid_value
                              , const petri_net::pid_t & pid_increment
                              , const pnet_t::input_t & input
                              , const pnet_t::output_descr_t &
                              )
{
//  ++cnt_trans;

  map_t m;

  for ( pnet_t::input_t::const_iterator it (input.begin())
      ; it != input.end()
      ; ++it
      )
    m[Function::Transition::get_pid<token_t>(*it)]
      = Function::Transition::get_token<token_t>(*it);

  pnet_t::output_t output;

  output.push_back (top_t (m[pid_value] + m[pid_increment], pid_value));
  output.push_back (top_t (m[pid_increment], pid_increment));

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

  petri_net::pid_t tid_step
    ( net.add_transition
      ( transition_t
        ( "step"
        , "${" + fhg::util::show (pid_value) + "}" + " < " + fhg::util::show (max)
        )
      )
    );
  petri_net::pid_t tid_break
    ( net.add_transition
      ( transition_t
        ( "break"
        , "${" + fhg::util::show(pid_value) + "}" + " >= " + fhg::util::show (max)
        )
      )
    );

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

  net.set_transition_function ( tid_break
                              , Function::Transition::Pass<token_t>()
                              );

  net.put_token (pid_value, 0);
  net.put_token (pid_increment, 1);

  marking (net);

  {
    Timer_t timer ("fire", max + 1);

    while (!net.enabled_transitions().empty())
      net.fire(net.enabled_transitions().first());
  }

  marking (net);

  return EXIT_SUCCESS;
}
