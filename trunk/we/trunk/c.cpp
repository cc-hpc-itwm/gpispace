
#include <net.hpp>

#include <iostream>
#include <cstdlib>

#include <tr1/unordered_map>

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

static token_t trans ( const petri_net::pid_t &
                     , const token_input_t & token_input
                     , const place_via_edge_t &
                     )
{
  return inc (Function::Transition::get_token<token_t> (token_input));
}

static bool cond_rem ( const pnet_t & net
                     , const token_t & rem
                     , const token_t & token
                     , const petri_net::pid_t & pid
                     , const petri_net::eid_t &
                     )
{
  place_t place (net.place (pid));
  
  return (token == ((shift(place) + rem) % branch_factor));
}

typedef std::tr1::unordered_map<petri_net::pid_t,token_t> capacity_map_t;

static bool cond_capacity ( const pnet_t & net
                          , const capacity_map_t & capacity
                          , const petri_net::pid_t & pid
                          , const petri_net::eid_t &
                          )
{
  capacity_map_t::const_iterator c (capacity.find(pid));

  if (c == capacity.end())
    throw std::runtime_error ("cond_capacity: capacity not found");

  return (net.num_token (pid) < c->second);
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
using petri_net::eid_t;
using petri_net::connection_t;
using petri_net::PT;
using petri_net::TP;

int
main ()
{
  pnet_t n;

  petri_net::pid_t pid[branch_factor];
  cnt_place_t p (0);

  capacity_map_t capacity;

  for (token_t rem (0); rem < branch_factor; ++rem)
    {
      pid[rem] = n.add_place (place_t (p++,rem));

      capacity[pid[rem]] = branch_factor * (branch_factor + 1);

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
            , & trans
            )
          , Function::Condition::In::Generic<token_t> 
            ( boost::bind (&cond_rem, boost::ref(n), rem, _1, _2, _3)
            )
          , Function::Condition::Out::Generic<token_t> 
            ( boost::bind ( &cond_capacity
                          , boost::ref(n)
                          , boost::ref(capacity)
                          , _1
                          , _2
                          )
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

  //  n.update_in_enabled();
  //  n.update_out_enabled();

  cout << "ENABLED INPUTS :: Transition -> (Place -> [Token via Edge])" << endl;

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    {
      pnet_t::pid_in_map_t m (n.in_map[*t]);

      cout << "Transition " << *t 
           << ": can_fire_by_input = "
           << ((n.in_enabled.find(*t) != n.in_enabled.end()) ? "true" : "false")
           << ":" << endl;

      for (pnet_t::pid_in_map_t::const_iterator i (m.begin()); i != m.end(); ++i)
        {
          cout << "Place " << i->first 
               << " [" << i->second.size() << "]"
               << ":";

          for ( std::vector<std::pair<token_t,eid_t> >::const_iterator k (i->second.begin())
              ; k != i->second.end()
              ; ++k
              )
            cout << " {" << k->first << " via " << k->second << "}";

          cout << endl;
        }
    }

  cout << "ENABLED OUTPUTS :: Transition -> [Place via Edge]" << endl;

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    {
      pnet_t::output_descr_t output_descr (n.out_map[*t]);

      cout << "Transition " << *t 
           << ": can_fire_by_output = "
           << ((n.out_enabled.find(*t) != n.out_enabled.end()) ? "true" : "false")
           << ":";

      for ( pnet_t::output_descr_t::const_iterator i (output_descr.begin())
          ; i != output_descr.end()
          ; ++i
          )
        cout << " {" << i->first << " via " << i->second << "}";

      cout << endl;
    }  

  cout << "new_enabled = [";

  for ( pnet_t::enabled_t::const_iterator it (n.new_enabled.begin())
      ; it != n.new_enabled.end()
      ; ++it
      )
    cout << " " << *it;

  cout << "]" << endl;

  return EXIT_SUCCESS;
}
