#ifndef DRTS_MASTER_HPP
#define DRTS_MASTER_HPP 1

#include <string>
#include <ctime>
#include <boost/thread.hpp>

namespace drts
{
  class Master
  {
    typedef boost::mutex mutex_type;
    typedef boost::condition_variable condition_type;
    typedef boost::unique_lock<mutex_type> lock_type;

  public:
    enum state_code
      {
        CONNECTED = 0
      , NOT_CONNECTED
      };

    explicit Master(std::string const& name);
    ~Master () {}

    inline bool is_connected () const
    {
      return m_state == CONNECTED;
    }

    inline void is_connected (bool b)
    {
      m_state = b ? CONNECTED : NOT_CONNECTED;
    }

    std::string const & name() const { return m_name; }

    void update_recv();
    void update_send();

    time_t last_send() const { return m_last_send; }
    time_t last_recv() const { return m_last_recv; }
    time_t last_job_recv() const { return m_last_job_recv; }

    bool is_polling () const { return m_polling; }
  private:
    mutable mutex_type m_stats_mutex;

    // disallow copy construction
    Master(Master const & other);
    Master & operator=(Master const & other);

    std::string m_name;
    state_code m_state;
    time_t m_last_recv;
    time_t m_last_send;
    time_t m_last_job_recv;

    size_t m_num_send;
    size_t m_num_recv;
    size_t m_num_jobs_recv;

    bool m_polling;
    time_t m_poll_interval;
  };
}

#endif // DRTS_MASTER_HPP
