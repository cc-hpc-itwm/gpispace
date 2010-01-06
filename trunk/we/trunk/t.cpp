
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <net.hpp>

typedef std::string place_t;
typedef std::string transition_t;
typedef std::string edge_t;
typedef std::string token_t;

typedef net<place_t, transition_t, edge_t, token_t> pnet_t;

using std::cout;
using std::endl;

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

static void print_net (const pnet_t & n)
{
  static unsigned int print_net_count (0);

  cout << "##### PRINT NET: " << print_net_count++ << endl;

  cout << "*** by transition" << endl;

  for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
    {
      cout << trans (n, *t) << endl;

      for ( pnet_t::adj_place_const_it pit (n.out_of_transition(*t))
          ; pit.has_more()
          ; ++pit
          )
        cout << " >>-{" << edge (n, pit()) << "}->> " << place (n, *pit) << endl;

      for ( pnet_t::adj_place_const_it pit (n.in_to_transition(*t))
          ; pit.has_more()
          ; ++pit
          )
        cout << " <<-{" << edge (n, pit()) << "}-<< " << place (n, *pit) << endl;
    }    

  cout << "*** by place" << endl;

  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << place (n, *p) << endl;

      for ( pnet_t::adj_transition_const_it tit (n.out_of_place(*p))
          ; tit.has_more()
          ; ++tit
          )
        cout << " >>-{" << edge (n, tit()) << "}->> " << trans (n, *tit) << endl;
      
      for ( pnet_t::adj_transition_const_it tit (n.in_to_place(*p))
          ; tit.has_more()
          ; ++tit
          )
        cout << " <<-{" << edge (n, tit()) << "}-<< " << trans (n, *tit) << endl;
    }    

  cout << "*** by edges" << endl;

  for (pnet_t::edge_const_it e (n.edges()); e.has_more(); ++e)
    {
      cout << *e << ":";

      pnet_t::pid_t pid;
      pnet_t::tid_t tid;

      const pnet_t::edge_type et (n.get_edge_info (*e, pid, tid));

      cout << " -- typ: " << ((et == pnet_t::PT) ? "PT" : "TP");
      cout << ", place: " << place (n, pid);
      cout << " " << ((et == pnet_t::PT) ? "-->" : "<--") << " ";
      cout << "transition: " << trans (n, tid);
      cout << endl;
    }

//   cout << "*** tokens" << endl;

//   for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
//     {
//       cout << "on " << place (n, *p) << ":";

//       for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
//         cout << " " << *tp;

//       cout << endl;
//     }

  cout << n;
}

int
main ()
{
  pnet_t n(5,4);

  cout << "add_place (readyL) => " << n.add_place ("readyL") << endl;
  cout << "add_place (readyR) => " << n.add_place ("readyR") << endl;
  cout << "add_place (workL) => " << n.add_place ("workL") << endl;
  cout << "add_place (workR) => " << n.add_place ("workR") << endl;
  cout << "add_place (semaphore) => " << n.add_place ("semaphore") << endl;

  cout << "add_transition (enterL) => " << n.add_transition ("enterL") << endl;
  cout << "add_transition (enterR) => " << n.add_transition ("enterR") << endl;
  cout << "add_transition (leaveL) => " << n.add_transition ("leaveL") << endl;
  cout << "add_transition (leaveR) => " << n.add_transition ("leaveR") << endl;

  cout << "add_edge_place_to_transition (e_wr_lr) => "
       << n.add_edge_place_to_transition("e_wr_lr","workR","leaveR") << endl;
  cout << "add_edge_place_to_transition (e_rr_er) => "
       << n.add_edge_place_to_transition("e_rr_rr","readyR","enterR") << endl;

  cout << "add_edge_place_to_transition (e_wl_ll) => "
       << n.add_edge_place_to_transition("e_wl_ll","workL","leaveL") << endl;
  cout << "add_edge_place_to_transition (e_rl_el) => "
       << n.add_edge_place_to_transition("e_rl_rl","readyL","enterL") << endl;

  cout << "add_edge_place_to_transition (e_s_el) => "
       << n.add_edge_place_to_transition("e_s_el","semaphore","enterL") << endl;
  cout << "add_edge_place_to_transition (e_s_er) => "
       << n.add_edge_place_to_transition("e_s_er","semaphore","enterR") << endl;

  cout << "add_edge_transition_to_place (e_el_wl) => "
       << n.add_edge_transition_to_place ("e_el_wl", "enterL", "workL") << endl;
  cout << "add_edge_transition_to_place (e_er_wr) => "
       << n.add_edge_transition_to_place ("e_er_wr", "enterR", "workR") << endl;

  cout << "add_edge_transition_to_place (e_ll_rl) => "
       << n.add_edge_transition_to_place ("e_ll_rl", "leaveL", "readyL") << endl;
  cout << "add_edge_transition_to_place (e_lr_rr) => "
       << n.add_edge_transition_to_place ("e_lr_rr", "leaveR", "readyR") << endl;

  cout << "add_edge_transition_to_place (e_ll_s) => "
       << n.add_edge_transition_to_place ("e_ll_s", "leaveL", "semaphore") << endl;
  cout << "add_edge_transition_to_place (e_lr_s) => "
       << n.add_edge_transition_to_place ("e_lr_s", "leaveR", "semaphore") << endl;

  print_net (n);

  cout << "delete_edge (e_s_er) => " <<  n.delete_edge ("e_s_er") << endl;
  cout << "delete_edge (e_lr_s) => " <<  n.delete_edge ("e_lr_s") << endl;
  cout << "delete_place (semaphore) => " << n.delete_place ("semaphore") << endl;
  cout << "delete_transition (enterL) => " << n.delete_transition ("enterL") << endl;

  print_net (n);

  // reconstruct

  // first add stuff that was deleted explicitely
  cout << "add_place (semaphore) => " << n.add_place ("semaphore") << endl;
  cout << "add_transition (enterL) => " << n.add_transition ("enterL") << endl;

  cout << "add_edge_place_to_transition (e_s_er) => "
       << n.add_edge_place_to_transition("e_s_er","semaphore","enterR") << endl;
  cout << "add_edge_transition_to_place (e_lr_s) => "
       << n.add_edge_transition_to_place ("e_lr_s", "leaveR", "semaphore") << endl;

  // add edges, that were deleted implicitly by delete_place/transition

  // implicitely deleted via deleting enterL
  cout << "add_edge_place_to_transition (e_rl_el) => "
       << n.add_edge_place_to_transition("e_rl_rl","readyL","enterL") << endl;
  cout << "add_edge_transition_to_place (e_el_wl) => "
       << n.add_edge_transition_to_place ("e_el_wl", "enterL", "workL") << endl;

  //  implicitly deleted via deleting semaphore
  cout << "add_edge_place_to_transition (e_s_el) => "
       << n.add_edge_place_to_transition("e_s_el","semaphore","enterL") << endl;
  cout << "add_edge_transition_to_place (e_ll_s) => "
       << n.add_edge_transition_to_place ("e_ll_s", "leaveL", "semaphore") << endl;

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
      pnet_t::pid_t pid;
      pnet_t::tid_t tid;

      const pnet_t::edge_type et (n.get_edge_info (*e, pid, tid));

      if (et == pnet_t::PT)
        {
          c.add_edge_place_to_transition ( n.edge(*e)
                                         , n.place(pid)
                                         , n.transition(tid)
                                         );
        }
      else
        {
          c.add_edge_transition_to_place ( n.edge(*e)
                                         , n.transition(tid)
                                         , n.place(pid)
                                         );
        }
    }

  print_net (c);

  cout << "#### MODIFIED" << endl;

  c.modify_place ("semaphore","Semaphore");
  c.replace_transition ("enterL", "t_enterL");
  c.replace_transition ("enterR", "t_enterR");
  c.modify_edge ("e_s_el","e_S_el");

  print_net (c);

  cout << "put_token (Semaphore,c) => " 
       << c.put_token ("Semaphore","c") << endl;
  cout << "put_token (readyL,p) => " 
       << c.put_token ("readyL","p") << endl;
  cout << "put_token (readyL,p) => " 
       << c.put_token ("readyL","p") << endl;
  cout << "put_token (readyL,p) => " 
       << c.put_token (c.get_place_id("readyL"),"p") << endl;
  cout << "put_token (readyL,q) => " 
       << c.put_token ("readyL","q") << endl;
  cout << "put_token (readyL,q) => " 
       << c.put_token ("readyL","q") << endl;
  cout << "put_token (readyL,q) => " 
       << c.put_token (c.get_place_id("readyL"),"q") << endl;
  cout << "put_token (readyR,p) => " 
       << c.put_token ("readyR","p") << endl;
  cout << "put_token (readyR,p) => " 
       << c.put_token (c.get_place_id("readyR"),"p") << endl;

  print_net (c);

  cout << "delete_one_token (readyL,p) => "
       << c.delete_one_token (c.get_place_id("readyL"),"p") << endl;
  cout << "delete_one_token (readyL,x) => "
       << c.delete_one_token ("readyL","x") << endl;
  cout << "delete_all_token (readyL,q) => "
       << c.delete_all_token ("readyL","q") << endl;
  cout << "delete_all_token (readyL,q) => "
       << c.delete_all_token ("readyL","q") << endl;

  print_net (c);

  cout << "num_token (readyL) => " << c.get_token("readyL").count() << endl;

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

  return EXIT_SUCCESS;
}
