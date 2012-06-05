// user defined transition function example, mirko.rahn@itwm.fraunhofer.de

#include <we/net_with_transition_function.hpp>

#include <cstdlib>

#include <iostream>
#include <sstream>

#include <string>

#include <boost/random.hpp>

using std::cout;
using std::endl;

typedef std::string place_t;
typedef std::string edge_t;
struct transition_t
{
public:
  std::string t;

  transition_t () : t("transition without a name") {}
  transition_t (const std::string & _t) : t(_t) {}

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP(t);
  }

  template<typename T>
  bool condition (T &) const { return true; }
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
typedef unsigned long token_t;

token_t inc (const token_t &);
token_t inc (const token_t & token) { return token + 1; }

typedef petri_net::net_with_transition_function< place_t
                                               , transition_t
                                               , edge_t
                                               , token_t
                                               > pnet_t;

static std::string brack (const std::string & x)
{
  std::ostringstream s; s << " [" << x << "]"; return s.str();
}

static std::string trans (const pnet_t & n, const petri_net::tid_t & t)
{
  std::ostringstream s; s << t << brack(n.get_transition (t).t); return s.str();
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

template<typename Engine>
static void fire_random_transition (pnet_t & n, Engine & engine)
{
  if (!n.enabled_transitions().empty())
    {
     petri_net::tid_t tid (n.enabled_transitions().random (engine));

      cout << "FIRE " << trans (n, tid) << " => ";

      n.fire (tid);
    }
};

using petri_net::connection_t;
using petri_net::edge::PT;
using petri_net::edge::TP;

int
main ()
{
  pnet_t n("test_u", 4,4);

  // a simple loop with up and down

  {
    petri_net::pid_t pid_top (n.add_place (place_t ("top")));
    petri_net::pid_t pid_down (n.add_place (place_t ("down")));
    petri_net::tid_t tid_top_down
      (n.add_transition (transition_t ("top_down")));
    n.set_transition_function
      (tid_top_down, Function::Transition::PassWithFun<token_t>(inc));
    petri_net::tid_t tid_down_top
      (n.add_transition (transition_t ("down_top")));
    n.set_transition_function
      (tid_down_top, Function::Transition::PassWithFun<token_t>(inc));

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
      (n.add_transition (transition_t ("top_down_pass")));
    n.set_transition_function
      (tid_top_down, Function::Transition::Pass<token_t>());
    petri_net::tid_t tid_down_top
      (n.add_transition (transition_t ("down_top_pass")));
    n.set_transition_function
      (tid_down_top, Function::Transition::Pass<token_t>());

    n.add_edge (edge_t ("top_out_pass"), connection_t (PT, tid_top_down, pid_top));
    n.add_edge (edge_t ("down_in_pass"), connection_t (TP, tid_top_down, pid_down));

    n.add_edge (edge_t ("down_out_pass"), connection_t (PT, tid_down_top, pid_down));
    n.add_edge (edge_t ("top_in_pass"), connection_t (TP, tid_down_top, pid_top));

    n.put_token (pid_top, 2718);
    n.put_token (pid_down, 3141);
   }

  marking (n);

  boost::mt19937 engine;

  for (int i(0); i < 50; ++i)
    {
      fire_random_transition (n, engine);
      marking (n);
    }

  return EXIT_SUCCESS;
}
