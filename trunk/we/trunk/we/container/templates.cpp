
#include "adjacency.ipp"

#include <we/type/id.hpp>

namespace adjacency
{
  template class table<petri_net::pid_t,petri_net::tid_t,petri_net::eid_t>;
  // as long as (tid_t `isTheSameType` pid_t) == True
  //  template class table<petri_net::tid_t,petri_net::pid_t,petri_net::eid_t>;
}
