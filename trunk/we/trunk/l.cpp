
// a net, consisting of k independent loops, mirko.rahn@itwm.fraunhofer.de

#include <net.hpp>
#include <timer.hpp>

#include <cstdlib>

#include <iostream>

#include <tr1/random>

typedef unsigned int loop_t;
typedef unsigned int id_t;
typedef std::pair<loop_t, id_t> node_t;

static const loop_t num_loops (2500);
static const id_t size_loop (15);
static const unsigned int num_rounds (50);
static const unsigned int num_fire (num_loops * size_loop * num_rounds);

#ifndef NMALLOCHOOK
#include <malloc.h>

/* Prototypes for our hooks.  */
static void my_init_hook (void);
static void *my_malloc_hook (size_t, const void *);
static void my_free_hook (void*, const void *);
     
/* Override initializing hook from the C library. */
void (*__malloc_initialize_hook) (void) = my_init_hook;

static void *(*old_malloc_hook)(size_t, const void *);
static void (*old_free_hook)(void *, const void *);

static void
my_init_hook (void)
{
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
}
     
static void *
my_malloc_hook (size_t size, const void *)
{
  void *result;
  /* Restore all old hooks */
  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  /* Call recursively */
  result = malloc (size);
  /* Save underlying hooks */
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  /* printf might call malloc, so protect it too. */
  printf ("malloc: %u %p\n", (unsigned int) size, result);
  /* Restore our own hooks */
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
  return result;
}
     
static void
my_free_hook (void *ptr, const void *)
{
  /* Restore all old hooks */
  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  /* Call recursively */
  free (ptr);
  /* Save underlying hooks */
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  /* printf might call free, so protect it too. */
  printf ("free: %p\n", ptr);
  /* Restore our own hooks */
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
}
#endif // NMALLOCHOOL

typedef node_t place_t;
typedef node_t transition_t;
typedef unsigned int edge_t;
typedef unsigned char token_t;

typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;

using std::cout;
using std::endl;

static void fire_random_transition (pnet_t & n, std::tr1::mt19937 & engine)
{
  petri_net::enabled_t t (n.enabled_transitions());

  if (!t.empty())
    {
      std::tr1::uniform_int<petri_net::enabled_t::size_type> 
        uniform (0,t.size()-1);

      n.fire (t.at(uniform (engine)));
    }
}

int
main ()
{
  {
    pnet_t n (num_loops * size_loop, num_loops * size_loop);

    std::tr1::mt19937 engine;

    {
      Timer_t timer ("construct net");

      for (loop_t l (0); l < num_loops; ++l)
        for (id_t i (0); i < size_loop; ++i)
          {
            n.add_place (node_t (l, i));
            n.add_transition (node_t (l, i));
          }

      edge_t e (0);

      for (loop_t l (0); l < num_loops; ++l)
        for (id_t i (0); i < size_loop; ++i)
          {
            n.add_edge 
              ( e++
              , petri_net::connection_t
                ( petri_net::PT
                , n.get_transition_id (node_t (l, i))
                , n.get_place_id (node_t (l, i))
                )
              );
            n.add_edge
              ( e++
              , petri_net::connection_t
                ( petri_net::TP
                , n.get_transition_id (node_t (l, i))
                , n.get_place_id (node_t (l, (i + 1) % size_loop))
                )
              );
          }
    }

    {
      Timer_t timer ("put token", num_loops);

      for (loop_t l (0); l < num_loops; ++l)
        {
          n.put_token (n.get_place_id (node_t (l, 0)), 'c');
        }
    }

    {
      unsigned int f (num_fire);

      Timer_t timer ("fire random transition", f);

      while (f--)
        fire_random_transition (n, engine);
    }

    std::cout << n.get_num_transitions() << std::endl;
    std::cout << n.get_num_places() << std::endl;
    std::cout << n.get_num_edges() << std::endl;
  }

  return EXIT_SUCCESS;
}
