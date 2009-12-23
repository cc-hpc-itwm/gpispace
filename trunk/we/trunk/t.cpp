
#include <cstdlib>
#include <iostream>
#include <string>

#include <net.hpp>

typedef std::string place_t;
typedef std::string transition_t;
typedef std::string edge_t;

typedef net<place_t, transition_t, edge_t> pnet;

using std::cout;
using std::endl;

static void print_net (const pnet & n)
{
  cout << "*** by transition" << endl;

  for (pnet::transition_it t (n.transitions()); t.has_more(); ++t)
    {
      cout << *t << ":" << endl;

      for ( pnet::adj_place_it pit (n.out_of_transition(*t))
          ; pit.has_more()
          ; ++pit
          )
        cout << " >>-{" << pit.get().second << "}->> " 
             << pit.get().first << endl;

      for ( pnet::adj_place_it pit (n.in_to_transition(*t))
          ; pit.has_more()
          ; ++pit
          )
        cout << " <<-{" << pit.get().second << "}-<< " 
             << pit.get().first << endl;
    }    

  cout << "*** by place" << endl;

  for (pnet::place_it p (n.places()); p.has_more(); ++p)
    {
      cout << *p << ":" << endl;

      for ( pnet::adj_transition_it tit (n.out_of_place(*p))
          ; tit.has_more()
          ; ++tit
          )
        cout << " >>-{" << tit.get().second << "}->> " 
             << tit.get().first << endl;
      
      for ( pnet::adj_transition_it tit (n.in_to_place(*p))
          ; tit.has_more()
          ; ++tit
          )
        cout << " <<-{" << tit.get().second << "}-<< " 
             << tit.get().first << endl;
    }    
}

int
main ()
{
  pnet n(12,4);

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

  print_net (n);

  return EXIT_SUCCESS;
}
