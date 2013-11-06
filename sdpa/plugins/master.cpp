#include "master.hpp"

namespace drts
{
  Master::Master(std::string const &a_name)
    : m_name (a_name)
    , m_state(Master::NOT_CONNECTED)

    , m_last_recv(boost::posix_time::from_time_t(0))
    , m_last_send(boost::posix_time::from_time_t(0))
    , m_last_job_recv(boost::posix_time::from_time_t(0))

    , m_num_send(0)
    , m_num_recv(0)
    , m_num_jobs_recv(0)
  {}

  void Master::update_recv()
  {
    lock_type lock(m_stats_mutex);
    ++m_num_recv;
    m_last_recv = boost::posix_time::microsec_clock::universal_time();
  }

  void Master::update_send()
  {
    lock_type lock(m_stats_mutex);
    ++m_num_send;
    m_last_send = boost::posix_time::microsec_clock::universal_time();
  }

  void Master::job_received()
  {
    lock_type lock(m_stats_mutex);
    ++m_num_jobs_recv;
    m_last_job_recv = boost::posix_time::microsec_clock::universal_time();
  }
}
