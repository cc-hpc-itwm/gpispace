
#include <iostream>
#include <string>
#include <net.hpp>
#include <cstdlib>
#include <malloc.h>

typedef unsigned int place_t;
typedef unsigned int transition_t;
typedef std::pair<unsigned int, unsigned int> pair_t;
typedef std::pair<pair_t, bool> edge_t;

using std::cout;
using std::endl;

static const unsigned int nplace (100);
static const unsigned int ntrans (100);
static const unsigned int factor (10);

int
main ()
{
  net<place_t, transition_t, edge_t> n(nplace, ntrans);

  for (unsigned int p(0); p < factor * nplace; ++p)
    n.add_place (p);

  for (unsigned int t(0); t < factor * ntrans; ++t)
    n.add_transition (t);

  for (unsigned int p(0); p < factor * nplace; ++p)
    for (unsigned int t(0); t < factor * ntrans; ++t)
      n.add_edge_place_to_transition(edge_t(pair_t(p, t), true), p, t);

  for (unsigned int t(0); t < factor * ntrans; ++t)
    for (unsigned int p(0); p < factor * nplace; ++p)
      n.add_edge_transition_to_place(edge_t(pair_t(t, p), false), t, p);

  malloc_stats();

  return EXIT_SUCCESS;
}
