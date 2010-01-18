
// user defined transition function example, mirko.rahn@itwm.fraunhofer.de

#include <net.hpp>

#include <cstdlib>

#include <iostream>
#include <sstream>

#include <string>

#include <tr1/random>

using std::cout;
using std::endl;

typedef std::string place_t;
typedef std::string edge_t;
typedef std::string transition_t;
typedef unsigned long token_t;

token_t inc (const token_t &);
token_t inc (const token_t & token) { return token + 1; }

typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;

static std::string brack (const std::string & x)
{
  std::ostringstream s; s << " [" << x << "]"; return s.str();
}

static std::string trans (const pnet_t & n, const petri_net::tid_t & t)
{
  std::ostringstream s; s << t << brack(n.transition (t)); return s.str();
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

static void fire_random_transition (pnet_t & n, std::tr1::mt19937 & engine)
{
  petri_net::enabled_t t (n.enabled_transitions());

  if (!t.empty())
    {
      std::tr1::uniform_int<petri_net::enabled_t::size_type>
        uniform (0,t.size()-1);

      petri_net::tid_t tid (t.at (uniform (engine)));

      cout << "FIRE " << trans (n, tid) << " => ";

      n.fire (tid);
    }
}

using petri_net::connection_t;
using petri_net::PT;
using petri_net::TP;

int
main ()
{
  pnet_t n(4,4);

  // a simple loop with up and down

  {
    petri_net::pid_t pid_top (n.add_place (place_t ("top")));
    petri_net::pid_t pid_down (n.add_place (place_t ("down")));
    petri_net::tid_t tid_top_down
      ( n.add_transition ( transition_t ("top_down")
                         , Function::Transition::PassWithFun<token_t>(inc)
                         )
      );
    petri_net::tid_t tid_down_top
      ( n.add_transition ( transition_t ("down_top")
                         , Function::Transition::PassWithFun<token_t>(inc)
                         )
      );

    n.add_edge (edge_t ("top_out"), connection_t (PT, tid_top_down, pid_top));
    n.add_edge (edge_t ("down_in"), connection_t (TP, tid_top_down, pid_down));

    n.add_edge (edge_t ("down_out"), connection_t (PT, tid_down_top, pid_down));
    n.add_edge (edge_t ("top_in"), connection_t (TP, tid_down_top, pid_top));

    n.put_token (pid_top, 0);
    n.put_token (pid_down, 1);
  }

  // a second loop, that just passes the tokens through

  {
    petri_net::pid_t pid_top (n.add_place (place_t ("top_pass")));
    petri_net::pid_t pid_down (n.add_place (place_t ("down_pass")));
    petri_net::tid_t tid_top_down
      ( n.add_transition ( transition_t ("top_down_pass")
                         , Function::Transition::Pass<token_t>()
                         )
      );
    petri_net::tid_t tid_down_top
      ( n.add_transition ( transition_t ("down_top_pass")
                         , Function::Transition::Pass<token_t>()
                         )
      );

    n.add_edge (edge_t ("top_out_pass"), connection_t (PT, tid_top_down, pid_top));
    n.add_edge (edge_t ("down_in_pass"), connection_t (TP, tid_top_down, pid_down));

    n.add_edge (edge_t ("down_out_pass"), connection_t (PT, tid_down_top, pid_down));
    n.add_edge (edge_t ("top_in_pass"), connection_t (TP, tid_down_top, pid_top));

    n.put_token (pid_top, 2718);
    n.put_token (pid_down, 3141);
   }

  marking (n);

  std::tr1::mt19937 engine;

  for (int i(0); i < 50; ++i)
    {
      fire_random_transition (n, engine);
      marking (n);
    }

  return EXIT_SUCCESS;
}
