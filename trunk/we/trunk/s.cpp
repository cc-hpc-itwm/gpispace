
// how to use named edges to swap two tokens, mirko.rahn@itwm.fraunhofer.de

#include <net.hpp>

#include <cstdlib>

#include <iostream>

#include <string>

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

  const map_t swap (const map_t & in)
  {
    map_t out;

    map_t::const_iterator i0 (in.find(0));
    map_t::const_iterator i1 (in.find(1));

    if (i0 == in.end() || i1 == in.end())
      throw std::runtime_error ("missing edges in swap");

    out[2] = i1->second;
    out[3] = i0->second;

    return out;
  }

  typedef EdgesOnly<token_t, swap> Swap;
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
main ()
{
  pnet_t n(2,1);

  petri_net::pid_t pid_A (n.add_place (place_t ("A")));
  petri_net::pid_t pid_B (n.add_place (place_t ("B")));
  petri_net::tid_t tid (n.add_transition ( transition_t ("t")
                                         , TransitionFunction::Swap()
                                         )
                       );
  
  n.add_edge_place_to_transition (edge_t (0, "a"), pid_A, tid);
  n.add_edge_place_to_transition (edge_t (1, "b"), pid_B, tid);

  n.add_edge_transition_to_place (edge_t (2, "b"), tid, pid_A);
  n.add_edge_transition_to_place (edge_t (3, "a"), tid, pid_B);

  n.put_token (pid_A, 'a');
  n.put_token (pid_B, 'b');

  marking (n);

  fire (n); marking (n);
  fire (n); marking (n);
  fire (n); marking (n);

  return EXIT_SUCCESS;
}
