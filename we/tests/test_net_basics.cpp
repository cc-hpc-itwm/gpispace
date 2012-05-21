// demonstrate basic usage of the pnet interface, mirko.rahn@itwm.fraunhofer.de

#include <we/net.hpp>
#include "timer.hpp"

#include <cstdlib>
#include <ctype.h>

#include <iostream>
#include <sstream>

#include <string>

#include <boost/function.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/nvp.hpp>

#include <boost/random.hpp>

typedef std::string place_t;

struct transition_t
{
public:
  std::string name;

  transition_t () : name ("transition without a name") {}
  transition_t (const std::string & _name) : name (_name) {}

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP(name);
  }

  template<typename T>
  bool condition (const T & ) const { return true; }
};

inline std::ostream & operator << (std::ostream & s, const transition_t & t)
{
  return s << t.name;
}

inline std::size_t hash_value (const transition_t & t)
{
  boost::hash<std::string> h;

  return h (t.name);
}

inline bool operator == (const transition_t & x, const transition_t & y)
{
  return x.name == y.name;
}

typedef std::string edge_t;
typedef std::string token_t;

typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;

using std::cout;
using std::endl;

static std::string brack (const std::string & x)
{
  std::ostringstream s; s << " [" << x << "]"; return s.str();
}

static std::string trans (const pnet_t & n, const petri_net::tid_t & t)
{
  std::ostringstream s; s << t << brack(n.get_transition (t).name); return s.str();
}

static std::string show_place (const pnet_t & n, const petri_net::pid_t & p)
{
  std::ostringstream s; s << p << brack(n.get_place (p)); return s.str();
}

static std::string edge (const pnet_t & n, const petri_net::eid_t & e)
{
  std::ostringstream s; s << e << brack (n.get_edge (e)); return s.str();
}

static void print_enabled (const pnet_t & n)
{
  cout << "ENABLED INPUTS :: Transition -> (Place -> [Token via Edge])" << endl;

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    {
      pnet_t::pid_in_map_t m (n.get_pid_in_map(*t));

      cout << "Transition " << trans (n, *t)
           << " can_fire = " << (n.get_can_fire (*t) ? "true" : "false")
           << ":" << endl;

      for (pnet_t::pid_in_map_t::const_iterator i (m.begin()); i != m.end(); ++i)
        {
          cout << "  Place " << show_place (n, i->first)
               << " [" << i->second.size() << "]"
               << ":";

          for ( pnet_t::vec_token_via_edge_t::const_iterator k (i->second.begin())
              ; k != i->second.end()
              ; ++k
              )
            cout << " {" << k->first << " via " << edge (n, k->second) << "}";

          cout << endl;
        }
    }

  cout << "ENABLED OUTPUTS :: Transition -> [Place via Edge]" << endl;

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    {
      pnet_t::output_descr_t output_descr (n.get_output_descr(*t));

      cout << "Transition " << trans (n, *t)
           << " can_fire = " << (n.get_can_fire (*t) ? "true" : "false")
           << ":";

      for ( pnet_t::output_descr_t::const_iterator i (output_descr.begin())
          ; i != output_descr.end()
          ; ++i
          )
        cout << " {" << show_place (n, i->first) << " via " << edge (n, i->second) << "}";

      cout << endl;
    }
}

static void print_net (const pnet_t & n)
{
  static unsigned int print_net_count (0);

  cout << "##### PRINT NET: " << print_net_count++ << endl;

  cout << "*** by transition" << endl;

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    {
      cout << trans (n, *t)
           << " can_fire == " << (n.get_can_fire (*t) ? "true" : "false")
           << endl;

      for ( petri_net::adj_place_const_it pit (n.out_of_transition(*t))
          ; pit.has_more()
          ; ++pit
          )
        cout << " >>-{" << edge (n, pit()) << "}->> " << show_place (n, *pit) << endl;

      for ( petri_net::adj_place_const_it pit (n.in_to_transition(*t))
          ; pit.has_more()
          ; ++pit
          )
        cout << " <<-{" << edge (n, pit()) << "}-<< " << show_place (n, *pit) << endl;
    }

  cout << "*** by place" << endl;

  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << show_place (n, *p) << endl;

      for ( petri_net::adj_transition_const_it tit (n.out_of_place(*p))
          ; tit.has_more()
          ; ++tit
          )
        cout << " >>-{" << edge (n, tit()) << "}->> " << trans (n, *tit) << endl;

      for ( petri_net::adj_transition_const_it tit (n.in_to_place(*p))
          ; tit.has_more()
          ; ++tit
          )
        cout << " <<-{" << edge (n, tit()) << "}-<< " << trans (n, *tit) << endl;
    }

  cout << "*** by edges" << endl;

  for (pnet_t::edge_const_it e (n.edges()); e.has_more(); ++e)
    {
      cout << *e << ":";

      const petri_net::connection_t connection (n.get_edge_info (*e));

      cout << " -- typ: " << ((connection.type == petri_net::PT) ? "PT" : "TP");
      cout << ", place: " << show_place (n, connection.pid);
      cout << " " << ((connection.type == petri_net::PT) ? "-->" : "<--") << " ";
      cout << "transition: " << trans (n, connection.tid);
      cout << endl;
    }

  cout << "*** tokens" << endl;

  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "on " << show_place (n, *p) << ": ";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << "." << *tp;

      cout << endl;
    }
}

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "[" << n.get_place (*p) << ": ";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << "." << *tp;

      cout << "]";
    }
  cout << endl;
}

template<typename Engine>
static void fire_random_transition (pnet_t & n, Engine & engine)
{
  if (!n.enabled_transitions().empty())
    n.fire_random (engine);
};

static void step (pnet_t & n, unsigned long k)
{
  pnet_t::transition_const_it t (n.transitions());
  typedef std::vector<petri_net::tid_t> tid_vec_t;
  tid_vec_t tid;
  boost::mt19937 engine;
  boost::uniform_int<tid_vec_t::size_type> uniform(0, t.size()-1);

  for (; t.has_more(); ++t)
    tid.push_back (*t);

  while (k)
    {
      const petri_net::tid_t f (tid[uniform (engine)]);

      if (n.get_can_fire (f))
        {
          n.fire (f);
          --k;
          marking (n);
        }
    }
}

static void add_place (pnet_t & n, const place_t & place)
{
  cout << "add_place (" << place << ") => " << n.add_place (place) << endl;
}

static void delete_place (pnet_t & n, const place_t & place)
{
  cout << "delete_place (" << place << ") => "
       << n.delete_place (n.get_place_id (place))
       << endl;
}

static void delete_edge (pnet_t & n, const edge_t & edge)
{
  cout << "delete_edge (" << edge << ") => "
       << n.delete_edge (n.get_edge_id (edge))
       << endl;
}

static void delete_transition (pnet_t & n, const std::string & transition)
{
  cout << "delete_transition (" << transition << ") => "
       << n.delete_transition (n.get_transition_id (transition_t (transition)))
       << endl;
}

static void add_transition (pnet_t & n, const std::string & transition)
{
  cout << "add_transition (" << transition
       << ") => " << n.add_transition (transition_t (transition))
       << endl;
}

static void add_edge_place_to_transition ( pnet_t & n
                                         , const edge_t & edge
                                         , const place_t & place
                                         , const std::string & transition
                                         )
{
  cout << "add_edge_place_to_transition (" << edge << ") => "
       << n.add_edge ( edge
                     , petri_net::connection_t
                       ( petri_net::PT
                       , n.get_transition_id (transition_t (transition))
                       , n.get_place_id (place)
                       )
                     )
       << endl;
}

static void add_edge_transition_to_place ( pnet_t & n
                                         , const edge_t & edge
                                         , const std::string & transition
                                         , const place_t & place
                                         )
{
  cout << "add_edge_place_to_transition (" << edge << ") => "
       << n.add_edge ( edge
                     , petri_net::connection_t
                       ( petri_net::TP
                       , n.get_transition_id (transition_t (transition))
                       , n.get_place_id (place)
                       )
                     )
       << endl;
}

static void put_token (pnet_t & n, const place_t & place, const token_t & token)
{
  cout << "put_token (" << place << "," << token << ")" <<  endl;

  n.put_token (n.get_place_id (place), token);

  print_enabled (n);
}

static void delete_one_token (pnet_t & n, const place_t & place, const token_t & token)
{
  cout << "delete_one_token (" << place << "," << token << ") => ";

  try
    {
      cout << n.delete_one_token (n.get_place_id (place), token);
    }
  catch (multirel::exception::delete_non_existing_object & e)
    {
      cout << e.what();
    }

  cout << endl;

  print_enabled (n);
}

static void delete_all_token (pnet_t & n, const place_t & place, const token_t & token)
{
  cout << "delete_all_token (" << place << "," << token << ") => ";

  try
    {
      cout << n.delete_all_token (n.get_place_id (place), token);
    }
  catch (multirel::exception::delete_non_existing_object & e)
    {
      cout << e.what();
    }

  cout << endl;
  print_enabled (n);
}

int
main ()
{
  boost::mt19937 engine;
  pnet_t n("test_t", 5,4);

  add_place (n, "readyL");
  add_place (n, "readyR");
  add_place (n, "workL");
  add_place (n, "workR");
  add_place (n, "semaphore");

  add_transition (n, "enterL");
  add_transition (n, "enterR");
  add_transition (n, "leaveL");
  add_transition (n, "leaveR");

  add_edge_place_to_transition(n, "e_wr_lr","workR","leaveR");
  add_edge_place_to_transition(n, "e_rr_rr","readyR","enterR");
  add_edge_place_to_transition(n, "e_wl_ll","workL","leaveL");
  add_edge_place_to_transition(n, "e_rl_rl","readyL","enterL");
  add_edge_place_to_transition(n, "e_s_el","semaphore","enterL");
  add_edge_place_to_transition(n, "e_s_er","semaphore","enterR");

  add_edge_transition_to_place (n, "e_el_wl", "enterL", "workL");
  add_edge_transition_to_place (n, "e_er_wr", "enterR", "workR");
  add_edge_transition_to_place (n, "e_ll_rl", "leaveL", "readyL");
  add_edge_transition_to_place (n, "e_lr_rr", "leaveR", "readyR");
  add_edge_transition_to_place (n, "e_ll_s", "leaveL", "semaphore");
  add_edge_transition_to_place (n, "e_lr_s", "leaveR", "semaphore");

  print_net (n);

  delete_edge (n, "e_s_er");
  delete_edge (n, "e_lr_s");
  delete_place (n, "semaphore");
  delete_transition (n, "enterL");

  print_net (n);

  // reconstruct

  // first add stuff that was deleted explicitely
  add_place (n, "semaphore");
  add_transition (n, "enterL");

  add_edge_place_to_transition(n, "e_s_er","semaphore","enterR");
  add_edge_transition_to_place (n, "e_lr_s", "leaveR", "semaphore");

  // add edges, that were deleted implicitly by delete_place/transition

  // implicitely deleted via deleting enterL
  add_edge_place_to_transition(n, "e_rl_rl","readyL","enterL");
  add_edge_transition_to_place (n, "e_el_wl", "enterL", "workL");

  //  implicitly deleted via deleting semaphore
  add_edge_place_to_transition(n, "e_s_el","semaphore","enterL");
  add_edge_transition_to_place (n, "e_ll_s", "leaveL", "semaphore");

  print_net (n);

  // deep copy

  cout << "#### DEEP COPIED" << endl;

  pnet_t c("c", n.get_num_places(),n.get_num_transitions());

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    c.add_transition (n.get_transition(*t));

  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    c.add_place (n.get_place(*p));

  for (pnet_t::edge_const_it e (n.edges()); e.has_more(); ++e)
    {
      const petri_net::connection_t connection (n.get_edge_info (*e));

      c.add_edge ( n.get_edge (*e)
                 , petri_net::connection_t
                   ( connection.type
                   , c.get_transition_id (n.get_transition (connection.tid))
                   , c.get_place_id (n.get_place (connection.pid))
                   )
                 );
    }

  print_net (c);

  cout << "#### MODIFIED" << endl;

  c.modify_place (c.get_place_id("semaphore"),"Semaphore");
  c.replace_transition (c.get_transition_id (transition_t ("enterL")), transition_t ("t_enterL"));
  c.replace_transition (c.get_transition_id (transition_t ("enterR")), transition_t ("t_enterR"));
  c.modify_edge (c.get_edge_id ("e_s_el"),"e_S_el");

  print_net (c);

  put_token (c, "Semaphore","c");
  put_token (c, "readyL","p");
  put_token (c, "readyL","p");
  put_token (c, "readyL","p");
  put_token (c, "readyL","q");
  put_token (c, "readyL","q");
  put_token (c, "readyL","q");
  put_token (c, "readyR","p");
  put_token (c, "readyR","p");

  print_net (c);

  delete_one_token (c,"readyL","p");
  delete_one_token (c,"readyL","x");
  delete_all_token (c,"readyL","q");
  delete_all_token (c,"readyL","q");

  print_net (c);

  cout << "num_token (readyL) => " << c.num_token(c.get_place_id("readyL"))
       << endl;

  cout << "replace_one_token (readyL,p->m) => "
       << c.replace_one_token (c.get_place_id ("readyL"),"p","m") << endl;

  print_net (c);

  try
    {
      cout << "replace_one_token (readyL,p->m) => "
           << c.replace_one_token (c.get_place_id ("readyL"),"p","m") << endl;
    }
  catch (multirel::exception::delete_non_existing_object & e)
    {
      cout << e.what();
    }

  try
    {
      cout << "replace_one_token (readyL,p->m) => "
           << c.replace_one_token (c.get_place_id ("readyL"),"p","m") << endl;
    }
  catch (multirel::exception::delete_non_existing_object & e)
    {
      cout << e.what() << endl;
    }

  try
    {
      cout << "replace_one_token (readyL,q->m) => "
           << c.replace_one_token (c.get_place_id ("readyL"),"q","m") << endl;
    }
  catch (multirel::exception::delete_non_existing_object & e)
    {
      cout << e.what() << endl;
    }

  try
    {
      cout << "replace_all_token (readyL,q->m) => "
           << c.replace_all_token (c.get_place_id ("readyL"),"q","m") << endl;
    }
  catch (multirel::exception::delete_non_existing_object & e)
    {
      cout << e.what() << endl;
    }

  try
    {
      cout << "replace_all_token (readyR,p->n) => "
           << c.replace_all_token (c.get_place_id ("readyR"),"p","n") << endl;
    }
  catch (multirel::exception::delete_non_existing_object & e)
    {
      cout << e.what() << endl;
    }

  print_net (c);

  print_enabled (c);

  cout << "FIRE t_enterL" << endl;
  c.fire(c.get_transition_id (transition_t ("t_enterL")));

  print_enabled (c);

  cout << "FIRE leaveL" << endl;
  c.fire(c.get_transition_id (transition_t ("leaveL")));

  print_enabled (c);

  print_net (c);

  // check whether or not the updates are working correctly

  cout << "UPDATE" << endl; print_enabled (c);

  c.modify_place (c.get_place_id ("readyL"), "ReadyL");

  cout << endl; print_enabled (c);

  c.modify_edge (c.get_edge_id ("e_S_el"), "!e_S_el");

  cout << endl; print_enabled (c);

  c.replace_one_token (c.get_place_id ("ReadyL"), "", "i");

  cout << endl; print_enabled (c);

  c.replace_place (c.get_place_id ("workL"), "WorkL");

  cout << endl; print_enabled (c);

  c.replace_place (c.get_place_id ("WorkL"), "workL");

  cout << endl; print_enabled (c);

  c.replace_edge (c.get_edge_id ("e_el_wl"), "!e_el_wl");

  cout << endl; print_enabled (c);

  {
    Timer_t timer ("step", 100);
    step (c, 100);
  }

  {
    unsigned int num_token (10);

    Timer_t timer ("put_token", num_token);

    while (num_token--)
      c.put_token (c.get_place_id("Semaphore"));
  }

  {
    pnet_t d;

    std::ostringstream oss;

    {
      boost::archive::text_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP(c);
    }

    print_net (c);

    {
      std::istringstream iss(oss.str());
      boost::archive::text_iarchive ia (iss, boost::archive::no_header);
      ia >> BOOST_SERIALIZATION_NVP(d);
    }

    print_net (d);

    cout << oss.str() << endl;
    cout << "SERIALIZATION SIZE = " << oss.str().length() << endl;

    try
      {
        fire_random_transition (d, engine);
      }
    catch (petri_net::exception::no_such)
      {
        cout << "cannot fire, the functions are not de-serialized" << endl;
      }
  }

  {
    unsigned int num_fire (1000000);

    Timer_t timer ("fire random transition", num_fire);

    while (num_fire--)
      fire_random_transition (c, engine);
  }

  return EXIT_SUCCESS;
}
