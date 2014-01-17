#include "master.hpp"

namespace drts
{
  Master::Master(std::string const &a_name)
    : m_name (a_name)
    , _is_connected (false)
  {}

  void Master::update_recv()
  {
  }

  void Master::update_send()
  {
  }
}
