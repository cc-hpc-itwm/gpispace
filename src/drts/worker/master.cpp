#include "master.hpp"

namespace drts
{
  Master::Master(std::string const &a_name)
    : m_name (a_name)
    , _is_connected (false)

    , m_num_send(0)
    , m_num_recv(0)
    , m_num_jobs_recv(0)
  {}

  void Master::update_recv()
  {
    lock_type lock(m_stats_mutex);
    ++m_num_recv;
  }

  void Master::update_send()
  {
    lock_type lock(m_stats_mutex);
    ++m_num_send;
  }

  void Master::job_received()
  {
    lock_type lock(m_stats_mutex);
    ++m_num_jobs_recv;
  }
}
