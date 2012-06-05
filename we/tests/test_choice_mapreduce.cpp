// map reduce with multi-token condition, mirko.rahn@itwm.fraunhofer.de

// calculates $\sum_{i=0}^{n} i^2$, splits the sum into $n$ pieces,
// the worker are calculating $x \mapsto x^2$ the join sums up
// everything, the result must be zero, since the accumulator is
// initialized with -(the wanted result)

#include <we/net.hpp>
#include <we/function/trans.hpp>
#include <we/function/cond.hpp>

#include "timer.hpp"

#include <string>

#include <boost/function.hpp>
#include <boost/random.hpp>

#include <map>

/* ************************************************************************* */

static const unsigned int NUM_WORKER (4);

/* ************************************************************************* */

typedef signed long value_t;
typedef int package_id_t;

typedef std::pair<value_t,package_id_t> token_t;

static value_t get_value (const token_t & token)
{
  return token.first;
}

static package_id_t package_id (const token_t & token)
{
  return token.second;
}

static std::ostream & operator << (std::ostream & s, const token_t token)
{
  return s << get_value (token) << "." << package_id (token);
}

/* ************************************************************************* */

typedef unsigned int cnt_t;
static cnt_t cnt (0);
typedef std::pair<cnt_t,std::string> tagged_string_t;

static tagged_string_t mk_tagged_string (const std::string & name)
{
  return tagged_string_t (cnt++, name);
}

static std::ostream & operator << (std::ostream & s, const tagged_string_t ts)
{
  return s << ts.second;
}

typedef tagged_string_t place_t;
typedef tagged_string_t edge_t;

static place_t mk_place (const std::string & name) { return mk_tagged_string (name); }
static edge_t mk_edge (const std::string & name) { return mk_tagged_string (name); }

/* ************************************************************************* */

typedef std::map<std::string,unsigned long> cnt_map_t;
static cnt_map_t cnt_map;

/* ************************************************************************* */

struct transition_t
{
public:
  cnt_t cnt;
  std::string name;
  bool real_cond;

  transition_t ( const cnt_t & _cnt
               , const std::string _name
               , const bool _real_cond = false
               )
    : cnt (_cnt)
    , name (_name)
    , real_cond (_real_cond)
  {}

  bool condition (Function::Condition::Traits<token_t>::choices_t & choices) const
  {
    ++cnt_map["cond_join: called"];

    for ( ; choices.has_more(); ++choices)
      {
        ++cnt_map["cond_join: checked"];

        cross::iterator<Function::Condition::Traits<token_t>::pid_in_map_t>
          choice (*choices);

        if (!choice.has_more())
          throw std::runtime_error ("STRANGE: got a choice with no entries");

        const package_id_t id (package_id ((*choice).second.first));
        bool all_equal (true);

        for (++choice; choice.has_more() && all_equal; ++choice)
          all_equal = (id == package_id ((*choice).second.first));

        if (all_equal)
          {
            ++cnt_map["cond_join: accepted"];

            return true;
          }
        else
          ++cnt_map["cond_join: rejected"];
      }

    ++cnt_map["cond_join: rejected all"];

    return false;
  };
};

inline std::size_t hash_value (const transition_t & t)
{
  boost::hash<cnt_t> h;

  return h (t.cnt);
}

inline bool operator == (const transition_t & x, const transition_t & y)
{
  return x.cnt == y.cnt;
}

static transition_t mk_transition ( const std::string & name
                                  , const bool & real_cond = false
                                  )
{
  return transition_t (cnt++, name, real_cond);
}

/* ************************************************************************* */

typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;

/* ************************************************************************* */

using std::cout;
using std::endl;

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

/* ************************************************************************* */

static token_t work_fun (const token_t & token)
{
  ++cnt_map["work_fun"];

  value_t v (get_value (token));

  return token_t (v*v, package_id(token));
}

/* ************************************************************************* */

typedef Function::Transition::Traits<token_t>::token_on_place_t top_t;

/* ************************************************************************* */

static pnet_t::output_t
trans_start  ( const petri_net::pid_t & pid_split_input
             , const petri_net::pid_t & pid_output
             , const pnet_t::input_t & input
             , const pnet_t::output_descr_t & output_descr
             )
{
  ++cnt_map["trans_start"];

  if (input.size() != 1)
    throw std::runtime_error ("start needs exactly one input");

  if (output_descr.size() != 2)
    throw std::runtime_error ("start needs exactly two outputs");

  const token_t token
    (Function::Transition::get_token<token_t>(*(input.begin())));

  pnet_t::output_t output;

  output.push_back (top_t (token, pid_split_input));

  value_t v (get_value (token));
  value_t result ((v * (v + 1) * (2 * v + 1)) / 6);

  output.push_back (top_t (token_t (-result, package_id (token)), pid_output));

  return output;
}

/* ************************************************************************* */

static pnet_t::output_t
trans_split  ( const pnet_t::input_t & input
             , const pnet_t::output_descr_t & output_descr
             )
{
  ++cnt_map["trans_split"];

  if (input.size() != 1)
    throw std::runtime_error ("split needs exactly one input");

  if (!(output_descr.size() > 0))
    throw std::runtime_error ("split needs at least one output");

  token_t token (Function::Transition::get_token<token_t>(*(input.begin())));

  const pnet_t::output_descr_t::const_iterator begin (output_descr.begin());
  const pnet_t::output_descr_t::const_iterator end (output_descr.end());

  pnet_t::output_t output;

  pnet_t::output_descr_t::const_iterator it (begin);

  for (value_t v (0); v <= get_value (token); ++v, ++it)
    {
      if (it == end)
        it = begin;

      output.push_back ( top_t ( token_t (v, package_id (token))
                               , Function::Transition::get_pid<token_t> (*it)
                               )
                       );
    }

  for (; it != end; ++it)
    output.push_back ( top_t ( token_t (0, package_id (token))
                             , Function::Transition::get_pid<token_t> (*it)
                             )
                     );

  return output;
}

/* ************************************************************************* */

static pnet_t::output_t
trans_join ( const pnet_t::input_t & input
           , const pnet_t::output_descr_t & output_descr
           )
{
  ++cnt_map["trans_join"];

  if (output_descr.size() != 1)
    throw std::runtime_error ("join needs exactly one output");

  if (!(input.size() > 1))
    throw std::runtime_error ("join needs at least accumulator + one input");

  petri_net::pid_t pid_out
    (Function::Transition::get_pid<token_t> (*(output_descr.begin())));

  pnet_t::input_t::const_iterator it (input.begin());

  value_t v (get_value (Function::Transition::get_token<token_t> (*it)));
  const package_id_t id
    (package_id (Function::Transition::get_token<token_t> (*it)));

  for (++it; it != input.end(); ++it)
    {
      v += get_value (Function::Transition::get_token<token_t> (*it));

      if (id != package_id (Function::Transition::get_token<token_t> (*it)))
        throw std::runtime_error ("BAD: adding tokens of different packages");
    }

  pnet_t::output_t output;

  output.push_back (top_t (token_t (v, id), pid_out));

  return output;
}

/* ************************************************************************* */

using petri_net::connection_t;
using petri_net::PT;
using petri_net::TP;

int
main ()
{
  pnet_t net;

  petri_net::pid_t pid_input (net.add_place (mk_place ("input")));
  petri_net::pid_t pid_output (net.add_place (mk_place ("output")));
  petri_net::pid_t pid_split_input (net.add_place (mk_place ("split input")));

  petri_net::tid_t tid_start (net.add_transition (mk_transition ("start")));

  net.set_transition_function ( tid_start
                              , boost::bind ( &trans_start
                                            , boost::ref (pid_split_input)
                                            , boost::ref (pid_output)
                                            , _1
                                            , _2
                                            )
                              );

  net.add_edge (mk_edge ("get input"), connection_t (PT, tid_start, pid_input));
  net.add_edge (mk_edge ("put split input"), connection_t (TP, tid_start, pid_split_input));
  net.add_edge (mk_edge ("put accumulator"), connection_t (TP, tid_start, pid_output));

  petri_net::pid_t tid_split (net.add_transition (mk_transition ("step")));

  net.set_transition_function
    ( tid_split
    , Function::Transition::Generic<token_t> (trans_split)
    );

  petri_net::pid_t tid_join (net.add_transition (mk_transition ("sum", true)));

  net.set_transition_function
    ( tid_join
    , Function::Transition::Generic<token_t> (trans_join)
    );

  net.add_edge (mk_edge ("get input"), connection_t (PT, tid_split, pid_split_input));

  for (unsigned int w(0); w < NUM_WORKER; ++w)
    {
      petri_net::pid_t in (net.add_place (mk_place ("in")));
      petri_net::pid_t out (net.add_place (mk_place ("out")));

      petri_net::tid_t worker (net.add_transition (mk_transition ("work")));

      net.set_transition_function
        ( worker
        , Function::Transition::PassWithFun<token_t>(work_fun)
        );

      net.add_edge (mk_edge ("worker get"), connection_t (PT, worker, in));
      net.add_edge (mk_edge ("worker put"), connection_t (TP, worker, out));

      net.add_edge (mk_edge ("split put"), connection_t (TP, tid_split, in));
      net.add_edge (mk_edge ("join get"), connection_t (PT, tid_join, out));
    }

  net.add_edge (mk_edge ("put output"), connection_t (TP, tid_join, pid_output));
  net.add_edge (mk_edge ("put output"), connection_t (PT, tid_join, pid_output));

  net.put_token (pid_input, token_t (2,0));
  net.put_token (pid_input, token_t (8,1));
  net.put_token (pid_input, token_t (360,2));

  marking (net);

  {
    boost::mt19937 engine; // engine.seed(3141);

    Timer_t timer ("calculate net");

    while (!net.enabled_transitions().empty())
      {
//         if (net.enabled_transitions().elem (tid_join))
//           net.fire (tid_join);
//         else
          net.fire_random(engine);
        //        marking (net);
      }
  }

  marking (net);

  for ( cnt_map_t::const_iterator it (cnt_map.begin())
      ; it != cnt_map.end()
      ; ++it
      )
    cout << it->first << " => " << it->second << endl;

  return EXIT_SUCCESS;
}
