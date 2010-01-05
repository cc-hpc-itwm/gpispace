
#include <cstdlib>
#include <iostream>
#include <string>

#include <net.hpp>

typedef std::string place_t;
typedef std::string transition_t;
typedef std::string edge_t;
typedef std::string token_t;

typedef net<place_t, transition_t, edge_t, token_t> pnet_t;

using std::cout;
using std::endl;

static void print_net (const pnet_t & n)
{
  cout << "*** by transition" << endl;

  for (pnet_t::transition_it t (n.transitions()); t.has_more(); ++t)
    {
      cout << *t << ":" << endl;

      for ( pnet_t::adj_place_it pit (n.out_of_transition(*t))
          ; pit.has_more()
          ; ++pit
          )
        cout << " >>-{" << pit.get_edge() << "}->> " << *pit << endl;

      for ( pnet_t::adj_place_it pit (n.in_to_transition(*t))
          ; pit.has_more()
          ; ++pit
          )
        cout << " <<-{" << pit.get_edge() << "}-<< " << *pit << endl;
    }    

  cout << "*** by place" << endl;

  for (pnet_t::place_it p (n.places()); p.has_more(); ++p)
    {
      cout << *p << ":" << endl;

      for ( pnet_t::adj_transition_it tit (n.out_of_place(*p))
          ; tit.has_more()
          ; ++tit
          )
        cout << " >>-{" << tit.get_edge() << "}->> " << *tit << endl;
      
      for ( pnet_t::adj_transition_it tit (n.in_to_place(*p))
          ; tit.has_more()
          ; ++tit
          )
        cout << " <<-{" << tit.get_edge() << "}-<< " << *tit << endl;
    }    

  cout << "*** by edges" << endl;

  for (pnet_t::edge_it e (n.edges()); e.has_more(); ++e)
    {
      cout << *e << ":";

      place_t place;
      transition_t transition;

      const pnet_t::edge_type et (n.get_edge_info (*e, place, transition));

      cout << " -- typ: " << ((et == pnet_t::PT) ? "PT" : "TP");
      cout << ", place: " << place;
      cout << " " << ((et == pnet_t::PT) ? "-->" : "<--") << " ";
      cout << "transition: " << transition;
      cout << endl;
    }
}

// static void print_marking (const pnet_t & n)
// {
//   std::cout << "** marking:" << std::endl;

//   for (pnet_t::place_it p (n.places()); p.has_more(); ++p)
//     {
//       std::cout << "token on place " << *p << ":";

//       for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
//         std::cout << " " << *tp;

//       std::cout << std::endl;
//     }
// }

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

  cout << n;

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

  cout << n;

  // deep copy

  cout << "#### DEEP COPIED" << endl;

  pnet_t c(n.get_num_places(),n.get_num_transitions());

  for (pnet_t::transition_it t (n.transitions()); t.has_more(); ++t)
    c.add_transition (*t);

  for (pnet_t::place_it p (n.places()); p.has_more(); ++p)
    c.add_place (*p);

  for (pnet_t::edge_it e (n.edges()); e.has_more(); ++e)
    {
      place_t place;
      transition_t transition;

      const pnet_t::edge_type et (n.get_edge_info (*e, place, transition));

      if (et == pnet_t::PT)
        {
          c.add_edge_place_to_transition (*e, place, transition);
        }
      else
        {
          c.add_edge_transition_to_place (*e, transition, place);
        }
    }

  print_net (c);

  cout << c;

  return EXIT_SUCCESS;
}
