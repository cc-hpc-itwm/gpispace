
#include "adjacency.ipp"

#include <we/type/id.hpp>

namespace adjacency
{
  template class table<petri_net::place_id_type,petri_net::transition_id_type,petri_net::edge_id_type>;
  template class table<unsigned short, unsigned int, char>;
  // as long as (tid_t `isTheSameType` pid_t) == True
  //  template class table<petri_net::transition_id_type,petri_net::place_id_type,petri_net::edge_id_type>;
}
