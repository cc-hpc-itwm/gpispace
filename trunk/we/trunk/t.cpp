
#include <iostream>
#include <string>
#include <net.hpp>
#include <cstdlib>
#include <vector>

typedef std::string place_t;
typedef std::string transition_t;
typedef std::string edge_t;

using std::cout;
using std::endl;

int
main ()
{
  net<place_t, transition_t, edge_t> n;

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

//   const std::vector<place_t> adj = n.out("leaveR");

//   std::cout << "out(leaveR) =>";

//   for (std::vector<place_t>::const_iterator it(adj.begin()); it != adj.end(); ++it)
//     std::cout << " " << *it;

//   std::endl;

  return EXIT_SUCCESS;
}
