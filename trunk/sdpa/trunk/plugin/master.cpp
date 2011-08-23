#include "master.hpp"

namespace drts
{
  Master::Master(std::string const &a_name)
    : m_name (a_name)
    , m_state(Master::CONNECTED)
    , m_last_recv(0)
    , m_last_send(0)
    , m_num_sent(0)
    , m_num_recv(0)
    , m_polling(false)
    , m_poll_interval(0)
  {}
}
