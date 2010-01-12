
// user defined transition function example, mirko.rahn@itwm.fraunhofer.de

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <net.hpp>
#include <tr1/random>

#include <vector>

#include <boost/function.hpp>

using std::cout;
using std::endl;

typedef std::string place_t;
typedef std::string edge_t;
typedef std::string transition_t;
typedef unsigned long token_t;

static token_t token (0);

token_t plus (void) { return token++; }
token_t minus (void) { return token--; }

template<token_t F (void)>
class TransitionFunction
{
private:
  typedef FireTraits<token_t>::input_t input_t;

  typedef FireTraits<token_t>::output_descr_t output_descr_t;
  typedef std::pair<token_t, id_traits::pid_t> token_on_place_t;
  typedef FireTraits<token_t>::output_t output_t;

public:
  const output_t operator () ( const input_t &
                             , const output_descr_t & output_descr
                             ) const
  {
    output_t output;

    for ( output_descr_t::const_iterator it (output_descr.begin())
        ; it != output_descr.end()
        ; ++it
        )
      output.push_back (token_on_place_t (F(), it->first));

    return output;
  }
};

typedef net<place_t, transition_t, edge_t, token_t> pnet_t;

#if 0
static std::string brack (const std::string & x)
{
  std::ostringstream s; s << " [" << x << "]"; return s.str();
}

static std::string trans (const pnet_t & n, const pnet_t::tid_t & t)
{
  std::ostringstream s; s << t << brack(n.transition (t)); return s.str();
}

static std::string place (const pnet_t & n, const pnet_t::pid_t & p)
{
  std::ostringstream s; s << p << brack(n.place (p)); return s.str();
}

static std::string edge (const pnet_t & n, const pnet_t::eid_t & e)
{
  std::ostringstream s; s << e << brack (n.edge (e)); return s.str();
}
#endif

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
  pnet_t::enabled_t t (n.enabled_transitions());

  if (!t.empty())
    {
      std::tr1::uniform_int<pnet_t::enabled_t::size_type> 
        uniform (0,t.size()-1);

      n.fire (t.at(uniform (engine)));
    }
}

int
main ()
{
  pnet_t n;

  // a simple loop

  pnet_t::pid_t pid_top (n.add_place (place_t ("top")));
  pnet_t::pid_t pid_down (n.add_place (place_t ("down")));
  pnet_t::tid_t tid_top_down (n.add_transition (transition_t ("top_down"), TransitionFunction<plus>()));
  pnet_t::tid_t tid_down_top (n.add_transition (transition_t ("down_top"), TransitionFunction<minus>()));
  
  n.add_edge_place_to_transition (edge_t ("top_out"), pid_top, tid_top_down);
  n.add_edge_transition_to_place (edge_t ("down_in"), tid_top_down, pid_down);
  n.add_edge_place_to_transition (edge_t ("down_out"), pid_down, tid_down_top);
  n.add_edge_transition_to_place (edge_t ("top_in"), tid_down_top, pid_top);

  n.put_token (pid_top, token++);
  n.put_token (pid_down, token++);
  
  marking (n);

  std::tr1::mt19937 engine;

  for (int i(0); i < 10; ++i)
    {
      fire_random_transition (n, engine);
      marking (n);
    }

  return EXIT_SUCCESS;
}
