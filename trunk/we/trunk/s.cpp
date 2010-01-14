
// how to use named edges to swap two tokens, mirko.rahn@itwm.fraunhofer.de

#include <net.hpp>

#include <cstdlib>

#include <iostream>

#include <string>

#include <boost/bind.hpp>

using std::cout;
using std::endl;

typedef std::string place_t;
typedef std::string transition_t;
typedef unsigned char token_t;

typedef std::pair<unsigned int,std::string> edge_t;
typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;

namespace TransitionFunction
{
  typedef Traits<token_t>::edges_only_t map_t;

  static map_t swap (const map_t * const in)
  {
    map_t out;

    map_t::const_iterator i0 (in->find(0));
    map_t::const_iterator i1 (in->find(1));

    if (i0 == in->end() || i1 == in->end())
      throw std::runtime_error ("missing edges in swap");

    out[2] = i1->second;
    out[3] = i0->second;

    return out;
  }

  struct swap_descr
  {
  public:
    typedef std::pair<petri_net::eid_t, petri_net::eid_t> pair_t;
    const pair_t i;
    const pair_t o;
    swap_descr (const pair_t & _i, const pair_t & _o) : i (_i), o (_o) {}
  };

  static map_t swap_state (const swap_descr * descr, const map_t * const in)
  {
    map_t out;

    map_t::const_iterator first  (in->find(descr->i.first));
    map_t::const_iterator second (in->find(descr->i.second));

    if (first == in->end() || second == in->end())
      throw std::runtime_error ("missing edges in swap");

    out[descr->o.first ] = second->second;
    out[descr->o.second] = first ->second;

    return out;
  }
}

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

static void fire (pnet_t & n)
{
  pnet_t::enabled_t t (n.enabled_transitions());

  assert (t.size() == 1);

  n.fire (t.at(0));
}

int
main (int argc, char **)
{
  pnet_t n(2,1);

  petri_net::pid_t pid_A (n.add_place (place_t ("A")));
  petri_net::pid_t pid_B (n.add_place (place_t ("B")));
  petri_net::tid_t tid (n.add_transition (transition_t ("t")));

  petri_net::eid_t eid0
    (n.add_edge_place_to_transition (edge_t (0, "a"), pid_A, tid));
  petri_net::eid_t eid1
    (n.add_edge_place_to_transition (edge_t (1, "b"), pid_B, tid));
  petri_net::eid_t eid2
    (n.add_edge_transition_to_place (edge_t (2, "b"), tid, pid_A));
  petri_net::eid_t eid3
    (n.add_edge_transition_to_place (edge_t (3, "a"), tid, pid_B));

  if (argc > 0)
    {
      TransitionFunction::EdgesOnly<token_t> 
        transfun (&TransitionFunction::swap);

      n.set_transition_function (tid, transfun);
    }
  else
    {
      TransitionFunction::swap_descr swap_descr 
        ( TransitionFunction::swap_descr::pair_t (eid0, eid1)
        , TransitionFunction::swap_descr::pair_t (eid2, eid3)
        );

      TransitionFunction::EdgesOnly<token_t> 
        transfun (boost::bind(&TransitionFunction::swap_state, &swap_descr, _1));

      n.set_transition_function (tid, transfun);
    }

  n.put_token (pid_A, 'a');
  n.put_token (pid_B, 'b');

  marking (n);

  fire (n); marking (n);
  fire (n); marking (n);
  fire (n); marking (n);

  return EXIT_SUCCESS;
}
