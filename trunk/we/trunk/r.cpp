// generic permute tokens, mirko.rahn@itwm.fraunhofer.de

#include <net.hpp>

#include <cstdlib>

#include <iostream>
#include <iomanip>

#include <map>
#include <string>

#include <algorithm>

#include <boost/bind.hpp>

#include <tr1/random>

using std::cout;
using std::endl;

typedef unsigned int place_t;
typedef unsigned int transition_t;
typedef char token_t;

static const token_t elements[] = {'a','b','c'};
static const std::size_t k (sizeof (elements) / sizeof (*elements));
static const int num_fire (100);

typedef unsigned int edge_left_t;
typedef char edge_right_t;

typedef std::pair<edge_left_t,edge_right_t> edge_t;

typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;

typedef TransitionFunction::Traits<token_t>::input_t input_t;
typedef TransitionFunction::Traits<token_t>::output_descr_t output_descr_t;
typedef TransitionFunction::Traits<token_t>::output_t output_t;
typedef TransitionFunction::Traits<token_t>::token_on_place_t token_on_place_t;

typedef std::map<edge_right_t, token_t> map_t;

// independent of the permutation!
static output_t permute ( const pnet_t & net
                        , const input_t & input
                        , const output_descr_t & output_descr
                        )
{
  map_t m;

  for (input_t::const_iterator it (input.begin()); it != input.end(); ++it)
    {
      edge_t edge (net.edge (TransitionFunction::get_eid<token_t> (*it)));

      m[edge.second] = TransitionFunction::get_token<token_t> (*it);
    }

  output_t output;

  for ( output_descr_t::const_iterator it (output_descr.begin())
      ; it != output_descr.end()
      ; ++it
      )
    {
      edge_t edge (net.edge (TransitionFunction::get_eid<token_t> (*it)));

      map_t::iterator m_it (m.find (edge.second));

      if (m_it == m.end())
        throw std::runtime_error ("permute: missing input edge");

      output.push_back 
        ( token_on_place_t ( m_it->second
                           , TransitionFunction::get_pid<token_t> (*it)
                           )
        );

      m.erase (m_it);
    }

  if (!m.empty())
    throw std::runtime_error ("permute: missing output edge");

  return output;
}

static void marking (const pnet_t & n, const petri_net::tid_t & tid)
{
  petri_net::tid_t t (0);
  std::size_t j (0);

  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p, ++j)
    {
      if (j % k == 0)
        {
          cout << endl;
          cout << ((t++ == tid) ? "*" : " ") << " ";
        }

      cout << "[" << std::setw(2) << n.place (*p) << ":";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << " " << *tp;

      cout << "]";
    }
  cout << endl;
}

static std::size_t fac (void)
{
  std::size_t f (1);

  for (std::size_t i (1); i <= k; ++i)
    f *= i;

  return f;
}

static void fire_random_transition (pnet_t & n, std::tr1::mt19937 & engine)
{
  pnet_t::enabled_t t (n.enabled_transitions());

  assert (!t.empty());
  assert (t.size() == fac());
  
  std::tr1::uniform_int<pnet_t::enabled_t::size_type> uniform (0,t.size()-1);

  pnet_t::enabled_t::size_type tid (t.at(uniform (engine)));

  n.fire (tid);

  marking (n, tid);

  n.verify_enabled_transitions ();
}

int
main ()
{
  pnet_t n;

  place_t place (0);
  transition_t transition (0);
  edge_left_t edge (0);

  TransitionFunction::Generic<token_t>::F f ( boost::bind ( &permute
                                                          , boost::ref(n)
                                                          , _1
                                                          , _2
                                                          )
                                            );
  token_t perm[k];

  memcpy (perm, elements, sizeof(elements));

  do
    {
      petri_net::tid_t tid 
        ( n.add_transition ( transition++
                           , TransitionFunction::Generic<token_t>(f)
                           )
        );

      petri_net::pid_t pid[k];

      for (std::size_t i (0); i < k; ++i)
        {
          pid[i] = n.add_place (place++);
          n.put_token (pid[i], elements[i]);
        }

      for (std::size_t i (0); i < k; ++i)
        {
          n.add_edge_place_to_transition 
            (edge_t (edge++, elements[i]), pid[i], tid);

          n.add_edge_transition_to_place 
            (edge_t (edge++, perm[i]), tid, pid[i]);
        }
    }
  while (std::next_permutation (perm, perm + k));
 
  std::tr1::mt19937 engine;

  marking (n, transition);

  for (int i(0); i < num_fire; ++i)
    fire_random_transition (n, engine);

  return EXIT_SUCCESS;
}
