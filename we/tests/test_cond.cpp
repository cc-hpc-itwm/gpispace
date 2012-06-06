// demonstrate usage of condition functions, mirko.rahn@itwm.fraunhofer.de

#include <we/net_with_transition_function.hpp>
#include <we/util/cross.hpp>

#include <iostream>
#include <cstdlib>

#include <boost/unordered_map.hpp>

#include <boost/function.hpp>

// ************************************************************************* //

typedef unsigned int token_second_t;
typedef std::pair<char,token_second_t> token_t;
typedef unsigned int cnt_edge_t;
typedef std::pair<cnt_edge_t,petri_net::pid_t> edge_t;
typedef unsigned int cnt_place_t;
typedef std::pair<cnt_place_t,token_second_t> place_t;

// ************************************************************************* //

static const token_second_t branch_factor (3);

typedef Function::Transition::Traits<token_t> traits;
typedef traits::token_input_t token_input_t;
typedef traits::place_via_edge_t place_via_edge_t;

using std::cout;
using std::endl;

static std::ostream & operator << (std::ostream & s, const token_t & token)
{
  return s << ":" << token.first << "-" << token.second << ":";
}

static token_second_t shift (const place_t & place)
{
  return place.second;
}

// ************************************************************************* //

struct transition_t
{
public:
  token_second_t t;
  typedef boost::function<place_t (const petri_net::pid_t &)> translate_t;
  translate_t translate;

  transition_t ( const unsigned int & _t
               , const translate_t & _translate
               )
    : t (_t)
    , translate (_translate)
  {}

  bool
  condition (Function::Condition::Traits<token_t>::choices_t & choices) const
  {
    for ( ; choices.has_more(); ++choices)
      {
        bool all_ok (true);

        for ( Function::Condition::Traits<token_t>::choice_it_t choice (*choices)
            ; choice.has_more() && all_ok
            ; ++choice
            )
          {
            place_t place (translate ((*choice).first));
            token_t token ((*choice).second.first);

            all_ok = (token.second == ((shift(place) + t) % branch_factor));
          }

        if (all_ok)
          return true;
      }
    return false;
  }
};

inline std::size_t hash_value (const transition_t & t)
{
  boost::hash<unsigned int> h;

  return h(t.t);
}

inline bool operator == (const transition_t & x, const transition_t & y)
{
  return x.t == y.t;
}

// ************************************************************************* //

typedef petri_net::net_with_transition_function< place_t
                                               , transition_t
                                               , edge_t
                                               , token_t
                                               > pnet_t;

// ************************************************************************* //

// match by pid, means put it there where it comes from
template<typename T>
static petri_net::pid_t edge_descr (const T & x)
{
  return Function::Transition::get_pid<token_t> (x);
}

static token_t inc (const token_t & token)
{
  return token_t
    ( token.first
    , ((token.second + 1 >= branch_factor) ? 0 : (token.second + 1))
    );
}

static token_t trans ( const petri_net::pid_t & pid
                     , const token_input_t & token_input
                     , const place_via_edge_t & place_via_edge
                     )
{
  const token_t token (Function::Transition::get_token<token_t> (token_input));

  cout << "trans: "
       << " descr " << pid
       << " token " << token
       << " from {" << Function::Transition::get_pid<token_t> (token_input)
       << " via " << Function::Transition::get_eid<token_t> (token_input)
       << "} to " << inc (token)
       << " on {" <<  Function::Transition::get_pid<token_t> (place_via_edge)
       << " via " <<  Function::Transition::get_eid<token_t> (place_via_edge)
       << "}"
       << endl;

  return inc (token);
}

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "[" << n.get_place(*p).first
           << "-"
           << n.get_token(*p).size()
           << ":";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << " " << *tp;

      cout << "]";
    }
  cout << endl;
}

using petri_net::tid_t;
using petri_net::eid_t;
using petri_net::connection_t;
using petri_net::edge::PT;
using petri_net::edge::TP;

int
main ()
{
  pnet_t n;

  petri_net::pid_t pid[branch_factor];
  cnt_place_t p (0);

  for (token_second_t rem (0); rem < branch_factor; ++rem)
    {
      char c = 'a';

      pid[rem] = n.add_place (place_t (p++,rem));

      for (token_second_t t (0); t < branch_factor; ++t)
        for (token_second_t i (0); i < branch_factor; ++i)
          n.put_token (pid[rem], token_t(c++,i));
    }

  cnt_edge_t e (0);

  for (token_second_t rem (0); rem < branch_factor; ++rem)
    {
      const tid_t tid
        ( n.add_transition
          ( transition_t ( rem
                         , boost::bind (&pnet_t::get_place, boost::ref(n), _1)
                         )
          )
        );

      n.set_transition_function
        ( tid
        , Function::Transition::MatchWithFun<token_t,petri_net::pid_t>
          ( & edge_descr<token_input_t>
          , & edge_descr<place_via_edge_t>
          , & trans
          )
        );

      for (token_second_t t (0); t < branch_factor; ++t)
        {
          n.add_edge (edge_t (e++, pid[t]), connection_t (PT, tid, pid[t]));
          n.add_edge (edge_t (e++, pid[t]), connection_t (TP, tid, pid[t]));
        }
    }

  marking (n);

  for (token_second_t c (0); c < branch_factor * branch_factor; ++c)
    for (token_second_t t (0); t < branch_factor; ++t)
      {
        cout << endl; n.fire (t);

        marking (n);
      }

  return EXIT_SUCCESS;
}
