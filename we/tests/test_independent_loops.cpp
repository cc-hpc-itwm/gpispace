// a net, consisting of k independent loops, mirko.rahn@itwm.fraunhofer.de

#include <we/net_with_transition_function.hpp>
#include "timer.hpp"

#include <cstdlib>
#include <cstdio>

#include <iostream>
#include <sstream>

#include <boost/random.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <boost/serialization/nvp.hpp>

#include <boost/version.hpp>
#if BOOST_VERSION <= 103800
#  define DISABLE_BINARY_TESTS
#endif

typedef unsigned int loop_t;
typedef unsigned int id_t;

struct node_t
{
public:
  std::pair<loop_t, id_t> dat;

  node_t () : dat() {}
  node_t (loop_t l, id_t i) : dat (l,i) {}

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP(dat);
  }


  template<typename T> bool condition (const T &) const { return true; }
};

inline std::size_t hash_value (const node_t & t)
{
  boost::hash<loop_t> h;

  return h (t.dat.first);
}

inline std::size_t operator == (const node_t & x, const node_t & y)
{
  return x.dat == y.dat;
}

static const loop_t num_loops (2500);
static const id_t size_loop (15);
static const unsigned int num_rounds (5);

#ifdef MALLOCHOOK
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
#endif // MALLOCHOOK

typedef node_t place_t;
typedef node_t transition_t;
typedef unsigned int edge_t;
typedef unsigned char token_t;

typedef petri_net::net_with_transition_function< place_t
                                               , transition_t
                                               , edge_t
                                               , token_t
                                               > pnet_t;

using std::cout;
using std::endl;

template<typename Engine>
static void fire_random_transition (pnet_t & n, Engine & engine)
{
  if (not n.can_fire())
    {
      throw std::runtime_error ("net cannot fire");
    }
  else
    {
      n.fire_random (engine);
    }
};

int
main ()
{
  {
    pnet_t n (num_loops * size_loop, num_loops * size_loop);

    boost::mt19937 engine;

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
                ( petri_net::edge::PT
                , n.get_transition_id (node_t (l, i))
                , n.get_place_id (node_t (l, i))
                )
              );
            n.add_edge
              ( e++
              , petri_net::connection_t
                ( petri_net::edge::TP
                , n.get_transition_id (node_t (l, i))
                , n.get_place_id (node_t (l, (i + 1) % size_loop))
                )
              );
          }
    }

    static const int tokens_per_loop (10);

    {
      Timer_t timer ("put token", num_loops * tokens_per_loop);

      for (loop_t l (0); l < num_loops; ++l)
        for (int i (0); i < tokens_per_loop; ++i)
          n.put_token (n.get_place_id (node_t (l, 0)));
    }

#ifndef DISABLE_BINARY_TESTS
    {
      std::ostringstream oss;

      {
        Timer_t timer ("serialize: binary");

        boost::archive::binary_oarchive oa (oss, boost::archive::no_header);
        oa << BOOST_SERIALIZATION_NVP(n);
      }

      cout << "size serialize: binary: " << oss.str().length() << endl;

      {
        Timer_t timer ("deserialize: binary");

        std::istringstream iss(oss.str());
        boost::archive::binary_iarchive oa (iss, boost::archive::no_header);
        oa >> BOOST_SERIALIZATION_NVP(n);
      }
    }
#endif

    {
      std::ostringstream oss;

      {
        Timer_t timer ("serialize: text");

        boost::archive::text_oarchive oa (oss, boost::archive::no_header);
        oa << BOOST_SERIALIZATION_NVP(n);
      }

      cout << "size serialize: text: " << oss.str().length() << endl;

      {
        Timer_t timer ("deserialize: text");

        std::istringstream iss(oss.str());
        boost::archive::text_iarchive oa (iss, boost::archive::no_header);
        oa >> BOOST_SERIALIZATION_NVP(n);
      }
    }

    {
      std::ostringstream oss;

      {
        Timer_t timer ("serialize: xml");

        boost::archive::xml_oarchive oa (oss, boost::archive::no_header);
        oa << BOOST_SERIALIZATION_NVP(n);
      }

      cout << "size serialize: xml: " << oss.str().length() << endl;

      {
        Timer_t timer ("deserialize: xml");

        std::istringstream iss(oss.str());
        boost::archive::xml_iarchive oa (iss, boost::archive::no_header);
        oa >> BOOST_SERIALIZATION_NVP(n);
      }
    }

    {
      Timer_t timer ( "fire random transition"
                    , num_loops * size_loop * num_rounds
                    );

      for (unsigned int r (0); r < num_rounds; ++r)
        {
          for (unsigned int z (0); z < size_loop; ++z)
            {
              for (unsigned int l (0); l < num_loops; ++l)
                fire_random_transition (n, engine);

              cout << "." << std::flush;
            }

          cout << "#" << std::flush;
        }

      cout << endl;
    }

    cout << n.get_num_transitions() << endl;
    cout << n.get_num_places() << endl;
    cout << n.get_num_edges() << endl;
  }

  return EXIT_SUCCESS;
}
