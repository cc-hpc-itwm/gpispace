// some measurements with the pnet interface, mirko.rahn@itwm.fraunhofer.de

#include <we/net_with_transition_function.hpp>
#include <we/type/condition.hpp>
#include "timer.hpp"

#include <cstdlib>
#ifndef __APPLE__
// malloc.h is deprecated on OSX.
#include <malloc.h>
#else
// malloc_stats() is missing on OSX / FreeBSD / Solaris / ...
void malloc_stats() { }
#endif

#include <iostream>
#include <sstream>

#include <string>

#include <boost/random.hpp>

typedef unsigned int place_t;

struct transition_t
{
public:
  unsigned int t;

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP(t);
  }

  transition_t (const unsigned int & _t) : t (_t) {}

  template<typename T>
  bool condition (const T &) const { return true; }
};

inline std::size_t hash_value (const transition_t & t)
{
  boost::hash<unsigned int> h;

  return h (t.t);
}

inline std::size_t operator == (const transition_t & x, const transition_t & y)
{
  return x.t == y.t;
}

typedef std::pair<unsigned int, unsigned int> pair_t;
typedef std::pair<pair_t, bool> edge_t;
typedef unsigned int token_t;

namespace {
static const unsigned int nplace (200);
static const unsigned int ntrans (200);
static const unsigned int factor (10);
static const unsigned int ntoken (10);
static const unsigned int branch (25);
static const unsigned int num_fire (1000);

static const unsigned int bisize (1000000);
}

typedef petri_net::net_with_transition_function< place_t
                                               , transition_t
                                               , edge_t
                                               , token_t
                                               > pnet_t;

using std::cout;
using std::endl;

int
main ()
{
  {
    pnet_t n("test_b", nplace, ntrans);

    {
      Timer_t timer ("add places", factor * nplace);

      for (unsigned int p(0); p < factor * nplace; ++p)
        n.add_place (p);
    }

    {
      Timer_t timer ("add transitions", factor * ntrans);

      for (unsigned int t(0); t < factor * ntrans; ++t)
        n.add_transition (t);
    }

    boost::mt19937 engine;

    {
      boost::uniform_int<unsigned long> uniform (0, factor * ntrans - 1);

      unsigned int duplicates (0);

      Timer_t timer ( "add edges place -> transitions"
                    , factor * nplace * branch
                    );

      for (unsigned int p(0); p < factor * nplace; ++p)
        for (unsigned int t(0); t < branch; ++t)
          try
            {
              const unsigned int r (uniform (engine));
              const transition_t rand (r);

              n.add_edge ( edge_t(pair_t(p, r), true)
                         , petri_net::connection_t ( petri_net::PT
                                                   , n.get_transition_id (rand)
                                                   , p
                                                   )
                         );
              n.add_edge ( edge_t(pair_t(r, p), false)
                         , petri_net::connection_t ( petri_net::TP
                                                   , n.get_transition_id (rand)
                                                   , p
                                                   )
                         );
            }
          catch (bijection::exception::already_there)
            {
              ++duplicates;
            }

      cout << "duplicates = " << duplicates << endl;
    }

    {
      boost::uniform_int<place_t> uniform (0, factor * nplace - 1);

      unsigned int duplicates (0);

      Timer_t timer ( "add edges transitions -> place"
                    , factor * ntrans * branch
                    );

      for (unsigned int t(0); t < factor * ntrans; ++t)
        for (unsigned int p(0); p < branch; ++p)
          try
            {
              place_t rand (uniform (engine));
              n.add_edge( edge_t(pair_t(t, rand), false)
                        , petri_net::connection_t (petri_net::TP, t, rand)
                        );
            }
          catch (bijection::exception::already_there)
            {
              ++duplicates;
            }

      cout << "duplicates = " << duplicates << endl;
    }

    {
      Timer_t timer ("add tokens", factor * nplace * ntoken);

      for (unsigned int p(0); p < factor * nplace; ++p)
        for (unsigned int t(0); t < ntoken; ++t)
          n.put_token (p, 0);
    }

    {
      Timer_t timer ( "traverse places plus one level of transitions"
                    , factor * nplace
                    );

      unsigned int t_in (0);
      unsigned int t_out (0);
      unsigned int e_in (0);
      unsigned int e_out (0);

      for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
        {
          for ( petri_net::adj_transition_const_it tit (n.in_to_place(*p))
              ; tit.has_more()
              ; ++tit
              )
            {
              t_in += *tit;
              e_in += tit();
            }

          for ( petri_net::adj_transition_const_it tit (n.out_of_place(*p))
              ; tit.has_more()
              ; ++tit
              )
            {
              t_out += *tit;
              e_out += tit();
            }
        }

      cout << "t_in = " << t_in << endl;
      cout << "t_out = " << t_out << endl;
      cout << "e_in = " << e_in << endl;
      cout << "e_out = " << e_out << endl;
    }

    {
      Timer_t timer ("traverse transitions plus one level of places", factor * ntrans);

      unsigned int p_in (0);
      unsigned int p_out (0);
      unsigned int e_in (0);
      unsigned int e_out (0);

      for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
        {
          for ( petri_net::adj_place_const_it pit (n.in_to_transition(*t))
              ; pit.has_more()
              ; ++pit
              )
            {
              p_in += *pit;
              e_in += pit();
            }

          for ( petri_net::adj_place_const_it pit (n.out_of_transition(*t))
              ; pit.has_more()
              ; ++pit
              )
            {
              p_out += *pit;
              e_out += pit();
            }
        }

      cout << "p_in = " << p_in << endl;
      cout << "p_out = " << p_out << endl;
      cout << "e_in = " << e_in << endl;
      cout << "e_out = " << e_out << endl;
    }

    {
      Timer_t timer ("fire transitions", num_fire);
      unsigned int fired (0);

      for (unsigned int f(0); f < num_fire; ++f)
        {
          const pnet_t::enabled_t& t (n.enabled_transitions());

          if (!t.empty())
            {
              n.fire_random(engine);

              ++fired;
            }
        }

      cout << "fired = " << fired << endl;
    }

    malloc_stats();
  }

  malloc_stats();

  return EXIT_SUCCESS;
}
