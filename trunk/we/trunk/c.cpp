
#include <net.hpp>

#include <iostream>
#include <cstdlib>

#include <boost/function.hpp>

typedef unsigned int transition_t;
typedef unsigned int token_t;
typedef unsigned int cnt_edge_t;
typedef std::pair<cnt_edge_t,petri_net::pid_t> edge_t;
typedef unsigned int cnt_place_t;
typedef std::pair<cnt_place_t,token_t> place_t;

static const token_t branch_factor (3);

typedef petri_net::net<place_t,transition_t,edge_t,token_t> pnet_t;

typedef Function::Transition::Traits<token_t> traits;
typedef traits::token_input_t token_input_t;
typedef traits::place_via_edge_t place_via_edge_t;

static token_t shift (const place_t & place)
{
  return place.second;
}

template<typename T>
static petri_net::pid_t edge_descr (const T & x)
{
  return Function::Transition::get_pid<token_t> (x);
}

static token_t inc (const token_t & token)
{
  return ((token + 1 >= branch_factor) ? 0 : (token + 1));
}

static token_t transfun ( const petri_net::pid_t &
                        , const token_input_t & token_input
                        , const place_via_edge_t &
                        )
{
  return inc (Function::Transition::get_token<token_t> (token_input));
}

static bool cond_rem ( const pnet_t & net
                     , const token_t & rem
                     , const token_input_t & token_input
                     )
{
  petri_net::pid_t pid (Function::Transition::get_pid<token_t> (token_input));
  place_t place (net.place (pid));
  
  return (  Function::Transition::get_token<token_t>(token_input) 
         == ((shift(place) + rem) % branch_factor)
         );
}

using std::cout;
using std::endl;

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "[" << n.place(*p).first << ": ";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << " " << *tp;

      cout << "]";
    }
  cout << endl;
}

using petri_net::tid_t;
using petri_net::connection_t;
using petri_net::PT;
using petri_net::TP;

int
main ()
{
  pnet_t n;

  petri_net::pid_t pid[branch_factor];
  cnt_place_t p (0);

  for (token_t rem (0); rem < branch_factor; ++rem)
    {
      pid[rem] = n.add_place (place_t (p++,rem));

      for (token_t t (0); t < branch_factor; ++t)
        for (token_t i (0); i < branch_factor; ++i)
          n.put_token (pid[rem], i);
    }

  cnt_edge_t e (0);

  for (token_t rem (0); rem < branch_factor; ++rem)
    {
      const tid_t tid 
        ( n.add_transition 
          ( rem
          , Function::Transition::MatchWithFun<token_t,petri_net::pid_t>
            ( & edge_descr<token_input_t>
            , & edge_descr<place_via_edge_t>
            , & transfun
            )
          , Function::Condition::Pre::Generic<token_t> 
            ( boost::bind (&cond_rem, boost::ref(n), rem, _1)
            )
          )
        );

      for (token_t t (0); t < branch_factor; ++t)
        {
          n.add_edge (edge_t (e++, pid[t]), connection_t (PT, tid, pid[t]));
          n.add_edge (edge_t (e++, pid[t]), connection_t (TP, tid, pid[t]));
        }
    }
  
  marking (n);

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    {
      pnet_t::in_map_t m (n.en(*t));

      cout << "Transition " << *t << ":" << endl;

      for (pnet_t::in_map_t::const_iterator i (m.begin()); i != m.end(); ++i)
        {
          cout << "Place " << i->first << ":";

          for ( std::vector<token_t>::const_iterator k (i->second.begin())
              ; k != i->second.end()
              ; ++k
              )
            cout << " " << *k;

          cout << endl;
        }
    }

  return EXIT_SUCCESS;
}
