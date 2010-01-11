
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <malloc.h>

#include <net.hpp>
#include <timer.hpp>
#include <bijection.hpp>

#include <tr1/random>

typedef unsigned int place_t;
typedef unsigned int transition_t;
typedef std::pair<unsigned int, unsigned int> pair_t;
typedef std::pair<pair_t, bool> edge_t;
typedef unsigned int token_t;

static const unsigned int nplace (400);
static const unsigned int ntrans (100);
static const unsigned int factor (5);
static const unsigned int token (10);
static const unsigned int branch (25);
static const unsigned int num_fire (1000000);

static const unsigned int bisize (1000000);

typedef net<place_t, transition_t, edge_t, token_t> net_t;

int
main ()
{
  {
    net_t n(nplace, ntrans);

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

    std::tr1::mt19937 engine;

    {
      std::tr1::uniform_int<transition_t> uniform (0, factor * ntrans - 1);

      Timer_t timer ( "add edges place -> transitions"
                    , factor * nplace * branch
                    );

      for (unsigned int p(0); p < factor * nplace; ++p)
        for (unsigned int t(0); t < branch; ++t)
          try
            {
              transition_t rand (uniform (engine));
              n.add_edge_place_to_transition( edge_t(pair_t(p, rand), true)
                                            , p
                                            , rand
                                            );
            }
          catch (already_there)
            {
              /* std::cout << "duplicate" << std::endl; */
            }
    }

    {
      std::tr1::uniform_int<place_t> uniform (0, factor * nplace - 1);

      Timer_t timer ( "add edges transitions -> place"
                    , factor * ntrans * branch
                    );

      for (unsigned int t(0); t < factor * ntrans; ++t)
        for (unsigned int p(0); p < branch; ++p)
          try
            {
              place_t rand (uniform (engine));
              n.add_edge_transition_to_place( edge_t(pair_t(t, rand), false)
                                            , t
                                            , rand
                                            );
            }
          catch (already_there)
            {
              /* std::cout << "duplicate" << std::endl; */
            }
    }

    {
      Timer_t timer ("add tokens", factor * nplace * token);

      for (unsigned int p(0); p < factor * nplace; ++p)
        for (unsigned int t(0); t < token; ++t)
          n.put_token (p, 0);
    }

    {
      Timer_t timer ("traverse places plus one level of transitions", factor * nplace);

      unsigned int t_in (0);
      unsigned int t_out (0);
      unsigned int e_in (0);
      unsigned int e_out (0);

      for (net_t::place_const_it p (n.places()); p.has_more(); ++p)
        {
          for ( net_t::adj_transition_const_it tit (n.in_to_place(*p))
              ; tit.has_more()
              ; ++tit
              )
            {
              t_in += *tit;
              e_in += tit();
            }

          for ( net_t::adj_transition_const_it tit (n.out_of_place(*p))
              ; tit.has_more()
              ; ++tit
              )
            {
              t_out += *tit;
              e_out += tit();
            }
        }

      std::cout << "t_in = " << t_in << std::endl;
      std::cout << "t_out = " << t_out << std::endl;
      std::cout << "e_in = " << e_in << std::endl;
      std::cout << "e_out = " << e_out << std::endl;
    }

    {
      Timer_t timer ("traverse transitions plus one level of places", factor * ntrans);

      unsigned int p_in (0);
      unsigned int p_out (0);
      unsigned int e_in (0);
      unsigned int e_out (0);

      for (net_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
        {
          for ( net_t::adj_place_const_it pit (n.in_to_transition(*t))
              ; pit.has_more()
              ; ++pit
              )
            {
              p_in += *pit;
              e_in += pit();
            }

          for ( net_t::adj_place_const_it pit (n.out_of_transition(*t))
              ; pit.has_more()
              ; ++pit
              )
            {
              p_out += *pit;
              e_out += pit();
            }
        }

      std::cout << "p_in = " << p_in << std::endl;
      std::cout << "p_out = " << p_out << std::endl;
      std::cout << "e_in = " << e_in << std::endl;
      std::cout << "e_out = " << e_out << std::endl;
    }

    {
      Timer_t timer ("fire transitions", num_fire);
      unsigned int fired (0);

      for (unsigned int f(0); f < num_fire; ++f)
        {
          net_t::enabled_t t (n.enabled_transitions());

          if (!t.empty())
            {
              std::tr1::uniform_int<net_t::enabled_t::size_type>
                uniform (0, t.size() - 1);

              net_t::enabled_t::size_type pos (uniform(engine));

              n.fire (t.at (pos));

              ++fired;
            }
        }

      std::cout << "fired = " << fired << std::endl;
    }

    malloc_stats();
  }

  malloc_stats();

  return EXIT_SUCCESS;

  {
    bijection::bijection<place_t> bi;

    {
      Timer_t timer ("unsafe insert elements to bijection", bisize);
      
      for (unsigned int i(0); i < bisize; ++i)
        bi.unsafe_insert (i);
    }

    {
      Timer_t timer ("get_id from bijection", bisize);

      bijection::id_t s (0);
      
      for (unsigned int i(0); i < bisize; ++i)
        s += bi.get_id (i);

      std::cout << "s = " << s << std::endl;
    }

    {
      Timer_t timer ("get_elem from bijection", bisize);

      unsigned int s (0);
      
      for (bijection::id_t i(0); i < bisize; ++i)
        s += bi.get_elem (i);

      std::cout << "s = " << s << std::endl;
    }

    {
      Timer_t timer ("erase from bijection", bisize);

      for (unsigned int i(0); i < bisize; ++i)
        bi.erase (i);
    }

    malloc_stats();
  }

  {
    auto_bimap<place_t> bm("place_t");

    {
      Timer_t timer ("insert elements to auto_bimap", bisize);
      
      for (unsigned int i(0); i < bisize; ++i)
        bm.add (i);
    }

    {
      Timer_t timer ("get_id from auto_bimap", bisize);

      handle::T s (0);
      
      for (unsigned int i(0); i < bisize; ++i)
        s += bm.get_id (i);

      std::cout << "s = " << s << std::endl;
    }

    {
      Timer_t timer ("get_elem from auto_bimap", bisize);

      unsigned int s (0);
      
      for (handle::T i(0); i < bisize; ++i)
        s += bm.get_elem (i);

      std::cout << "s = " << s << std::endl;
    }

    {
      Timer_t timer ("erase from auto_bimap", bisize);

      for (unsigned int i(0); i < bisize; ++i)
        bm.erase (bm.get_id (i));
    }

    malloc_stats();
  }

  malloc_stats();

  return EXIT_SUCCESS;
}
