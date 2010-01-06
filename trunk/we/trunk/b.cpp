
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <malloc.h>
#include <sys/time.h>

#include <net.hpp>

typedef unsigned int place_t;
typedef unsigned int transition_t;
typedef std::pair<unsigned int, unsigned int> pair_t;
typedef std::pair<pair_t, bool> edge_t;
typedef unsigned int token_t;

static const unsigned int nplace (100);
static const unsigned int ntrans (100);
static const unsigned int factor (10);
static const unsigned int token (1000);

static inline double current_time()
{
  struct timeval tv;

  gettimeofday (&tv, NULL);

  return (double(tv.tv_sec) + double (tv.tv_usec) * 1E-6);
}

struct Timer_t
{
private:
  double t;
  const std::string msg;
  const unsigned int k;
public:
  Timer_t (const std::string _msg, const unsigned int _k = 1) 
    : t(-current_time())
    , msg(_msg)
    , k(_k)
  {}

  ~Timer_t ()
  {
    t += current_time();

    std::cout << "time " << msg 
              << " [" << k << "]: " 
              << t
              << " [" << t / double(k) << "]"
              << " [" << double(k) / t << "]"
              << std::endl;
  }
};

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

    {
      Timer_t timer ( "add edges place -> transitions"
                    , factor * nplace * factor * ntrans
                    );

      for (unsigned int p(0); p < factor * nplace; ++p)
        for (unsigned int t(0); t < factor * ntrans; ++t)
          n.add_edge_place_to_transition(edge_t(pair_t(p, t), true), p, t);
    }

    {
      Timer_t timer ( "add edges transitions -> place"
                    , factor * nplace * factor * ntrans
                    );

      for (unsigned int t(0); t < factor * ntrans; ++t)
        for (unsigned int p(0); p < factor * nplace; ++p)
          n.add_edge_transition_to_place(edge_t(pair_t(t, p), false), t, p);
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

    malloc_stats();
  }

  malloc_stats();

  return EXIT_SUCCESS;
}
