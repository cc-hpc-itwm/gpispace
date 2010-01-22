// demonstrate basic usage of the pnet interface, mirko.rahn@itwm.fraunhofer.de

#include <net.hpp>
#include <timer.hpp>

#include <cstdlib>
#include <ctype.h>

#include <iostream>
#include <sstream>

#include <string>

#include <boost/function.hpp>

#include <tr1/random>

typedef std::string place_t;
typedef std::string transition_t;
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
  std::ostringstream s; s << t << brack(n.transition (t)); return s.str();
}

static std::string place (const pnet_t & n, const petri_net::pid_t & p)
{
  std::ostringstream s; s << p << brack(n.place (p)); return s.str();
}

static std::string edge (const pnet_t & n, const petri_net::eid_t & e)
{
  std::ostringstream s; s << e << brack (n.edge (e)); return s.str();
}

static void print_enabled (const pnet_t & n)
{
  cout << "ENABLED INPUTS :: Transition -> (Place -> [Token via Edge])" << endl;

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    {
      pnet_t::pid_in_map_t m (n.in_map.find (*t)->second);

      cout << "Transition " << trans (n, *t)
           << ": can_fire_by_input = "
           << ((n.in_enabled.find(*t) != n.in_enabled.end()) ? "true" : "false")
           << ":" << endl;

      for (pnet_t::pid_in_map_t::const_iterator i (m.begin()); i != m.end(); ++i)
        {
          cout << "Place " << place (n, i->first)
               << " [" << i->second.size() << "]"
               << ":";

          for ( std::vector<std::pair<token_t,petri_net::eid_t> >::const_iterator k (i->second.begin())
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
      pnet_t::output_descr_t output_descr (n.out_map.find(*t)->second);

      cout << "Transition " << trans (n, *t)
           << ": can_fire_by_output = "
           << ((n.out_enabled.find(*t) != n.out_enabled.end()) ? "true" : "false")
           << ":";

      for ( pnet_t::output_descr_t::const_iterator i (output_descr.begin())
          ; i != output_descr.end()
          ; ++i
          )
        cout << " {" << place (n, i->first) << " via " << edge (n, i->second) << "}";

      cout << endl;
    }  

  cout << "new_enabled = [";

  for ( pnet_t::enabled_t::const_iterator it (n.new_enabled.begin())
      ; it != n.new_enabled.end()
      ; ++it
      )
    cout << " " << *it;

  cout << "]" << endl;
}

static void print_net (const pnet_t & n)
{
  static unsigned int print_net_count (0);

  cout << "##### PRINT NET: " << print_net_count++ << endl;

  cout << "*** by transition" << endl;

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    {
      cout << trans (n, *t)
           << " can_fire == " << (n.can_fire (*t) ? "true" : "false")
           << endl;

      for ( petri_net::adj_place_const_it pit (n.out_of_transition(*t))
          ; pit.has_more()
          ; ++pit
          )
        cout << " >>-{" << edge (n, pit()) << "}->> " << place (n, *pit) << endl;

      for ( petri_net::adj_place_const_it pit (n.in_to_transition(*t))
          ; pit.has_more()
          ; ++pit
          )
        cout << " <<-{" << edge (n, pit()) << "}-<< " << place (n, *pit) << endl;
    }

  cout << "*** by place" << endl;

  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << place (n, *p) << endl;

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
      cout << ", place: " << place (n, connection.pid);
      cout << " " << ((connection.type == petri_net::PT) ? "-->" : "<--") << " ";
      cout << "transition: " << trans (n, connection.tid);
      cout << endl;
    }

  cout << "*** tokens" << endl;

  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "on " << place (n, *p) << ": ";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << "." << *tp;

      cout << endl;
    }

  n.verify_enabled_transitions();
}

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "[" << n.place (*p) << ": ";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << "." << *tp;

      cout << "]";
    }
  cout << endl;

  n.verify_enabled_transitions();
}

static void fire_random_transition (pnet_t & n, std::tr1::mt19937 & engine)
{
  pnet_t::enabled_t t (n.enabled_transitions());

  assert (t == n.new_enabled);

  if (!t.empty())
    {
      std::tr1::uniform_int<pnet_t::enabled_t::size_type>
        uniform (0,t.size()-1);

      n.fire (t.at(uniform (engine)));
    }
}

static void step (pnet_t & n, unsigned long k)
{
  pnet_t::transition_const_it t (n.transitions());
  typedef std::vector<petri_net::tid_t> tid_vec_t;
  tid_vec_t tid;
  std::tr1::mt19937 engine;
  std::tr1::uniform_int<tid_vec_t::size_type> uniform(0, t.size()-1);

  for (; t.has_more(); ++t)
    tid.push_back (*t);

  while (k)
    {
      const petri_net::tid_t f (tid[uniform (engine)]);

      if (n.can_fire (f))
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

static void delete_transition (pnet_t & n, const transition_t & transition)
{
  cout << "delete_transition (" << transition << ") => "
       << n.delete_transition (n.get_transition_id (transition))
       << endl;
}

static void add_transition (pnet_t & n, const transition_t & transition)
{
  cout << "add_transition (" << transition
       << ") => " << n.add_transition (transition)
       << endl;
}

static void add_edge_place_to_transition ( pnet_t & n
                                         , const edge_t & edge
                                         , const place_t & place
                                         , const transition_t & transition
                                         )
{
  cout << "add_edge_place_to_transition (" << edge << ") => "
       << n.add_edge ( edge
                     , petri_net::connection_t
                       ( petri_net::PT
                       , n.get_transition_id (transition)
                       , n.get_place_id (place)
                       )
                     )
       << endl;
}

static void add_edge_transition_to_place ( pnet_t & n
                                         , const edge_t & edge
                                         , const transition_t & transition
                                         , const place_t & place
                                         )
{
  cout << "add_edge_place_to_transition (" << edge << ") => "
       << n.add_edge ( edge
                     , petri_net::connection_t
                       ( petri_net::TP
                       , n.get_transition_id (transition)
                       , n.get_place_id (place)
                       )
                     )
       << endl;
}

static void put_token (pnet_t & n, const place_t & place, const token_t & token)
{
  cout << "put_token (" << place << "," << token << ") => "
       << n.put_token (n.get_place_id (place), token)
       << endl;
  print_enabled (n);
}

static void delete_one_token (pnet_t & n, const place_t & place, const token_t & token)
{
  cout << "delete_one_token (" << place << "," << token << ") => "
       << n.delete_one_token (n.get_place_id (place), token)
       << endl;
  print_enabled (n);
}

static void delete_all_token (pnet_t & n, const place_t & place, const token_t & token)
{
  cout << "delete_all_token (" << place << "," << token << ") => "
       << n.delete_all_token (n.get_place_id (place), token)
       << endl;
  print_enabled (n);
}

static bool cond_in ( const pnet_t & net
                    , const token_t & token
                    , const petri_net::pid_t & pid
                    , const petri_net::eid_t & eid
                    )
{
  const place_t place (net.place (pid));
  const edge_t edge (net.edge (eid));

  return (isupper (place[0]) && (token.length() > 0 || edge[0] == '!'));
}

static bool cond_out ( const pnet_t & net
                     , const petri_net::pid_t & pid
                     , const petri_net::eid_t & eid
                     )
{
  const place_t place (net.place (pid));
  const edge_t edge (net.edge (eid));

  return (isupper (place[0]) || edge[0] == '!');
}

int
main ()
{
  pnet_t n(5,4);

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

  pnet_t c(n.get_num_places(),n.get_num_transitions());

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    c.add_transition (n.transition(*t));

  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    c.add_place (n.place(*p));

  for (pnet_t::edge_const_it e (n.edges()); e.has_more(); ++e)
    {
      const petri_net::connection_t connection (n.get_edge_info (*e));

      c.add_edge ( n.edge (*e)
                 , petri_net::connection_t
                   ( connection.type
                   , c.get_transition_id (n.transition (connection.tid))
                   , c.get_place_id (n.place (connection.pid))
                   )
                 );
    }

  print_net (c);

  cout << "#### MODIFIED" << endl;

  c.modify_place (c.get_place_id("semaphore"),"Semaphore");
  c.replace_transition (c.get_transition_id ("enterL"), "t_enterL");
  c.replace_transition (c.get_transition_id ("enterR"), "t_enterR");
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

  cout << "replace_one_token (readyL,p->m) => "
       << c.replace_one_token (c.get_place_id ("readyL"),"p","m") << endl;
  cout << "replace_one_token (readyL,p->m) => "
       << c.replace_one_token (c.get_place_id ("readyL"),"p","m") << endl;
  cout << "replace_one_token (readyL,q->m) => "
       << c.replace_one_token (c.get_place_id ("readyL"),"q","m") << endl;
  cout << "replace_all_token (readyL,q->m) => "
       << c.replace_all_token (c.get_place_id ("readyL"),"q","m") << endl;
  cout << "replace_all_token (readyR,p->n) => "
       << c.replace_all_token (c.get_place_id ("readyR"),"p","n") << endl;

  print_net (c);

  print_enabled (c);

  cout << "FIRE t_enterL" << endl;
  c.fire(c.get_transition_id ("t_enterL"));

  print_enabled (c);

  cout << "FIRE leaveL" << endl;
  c.fire(c.get_transition_id ("leaveL"));

  print_enabled (c);

  print_net (c);

  {
    Timer_t timer ("step", 100);
    step (c, 100);
  }

  std::tr1::mt19937 engine;

  {
    unsigned int num_fire (1000000);

    Timer_t timer ("fire random transition", num_fire);

    while (num_fire--)
      fire_random_transition (c, engine);
  }

  // check whether or not the updates areworking correctly

  cout << endl; print_enabled (c);

  c.set_in_condition_function ( c.get_transition_id ("t_enterR")
                              , boost::bind ( &cond_in
                                            , boost::ref (c)
                                            , _1
                                            , _2
                                            , _3
                                            )
                              );

  cout << endl; print_enabled (c);

  c.modify_place (c.get_place_id ("readyR"), "ReadyR");

  cout << endl; print_enabled (c);

  c.modify_edge (c.get_edge_id ("e_s_er"), "!e_s_er");

  cout << endl; print_enabled (c);

  c.replace_one_token (c.get_place_id ("ReadyR"), "", "i");

  cout << endl; print_enabled (c);

  c.set_out_condition_function ( c.get_transition_id ("t_enterR")
                               , boost::bind ( &cond_out
                                             , boost::ref (c)
                                             , _1
                                             , _2
                                             )
                               );

  cout << endl; print_enabled (c);

  cout << "++++" << endl;

  c.replace_place (c.get_place_id ("workR"), "WorkR");

  cout << endl; print_enabled (c);

  c.replace_place (c.get_place_id ("WorkR"), "workR");

  cout << endl; print_enabled (c);

  c.replace_edge (c.get_edge_id ("e_er_wr"), "!e_er_wr");

  cout << endl; print_enabled (c);

  return EXIT_SUCCESS;
}
