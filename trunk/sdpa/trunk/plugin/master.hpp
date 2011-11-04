#ifndef DRTS_MASTER_HPP
#define DRTS_MASTER_HPP 1

#include <string>
#include <list>
#include <ctime>

#include <fhg/util/thread/queue.hpp>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <sdpa/events/events.hpp>

namespace drts
{
  class Master
  {
    typedef boost::mutex mutex_type;
    typedef boost::condition_variable condition_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef boost::posix_time::ptime time_type;
    typedef boost::posix_time::time_duration time_duration;

    typedef fhg::thread::queue< sdpa::events::SDPAEvent::Ptr
                              , std::list
                              > event_queue_t;
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

    time_duration min_poll_interval() const { return m_min_poll_interval; }
    time_duration max_poll_interval() const { return m_max_poll_interval; }
    time_duration cur_poll_interval() const { return m_cur_poll_interval; }

    time_type last_send() const { return m_last_send; }
    time_type last_recv() const { return m_last_recv; }
    time_type last_job_recv() const { return m_last_job_recv; }
    time_type last_job_rqst() const { return m_last_job_rqst; }

    void job_received();
    void job_requested();

    bool is_polling () const { return m_polling; }
    void set_is_polling (bool b) { m_polling = b; }
    void reset_poll_rate();
    void decrease_poll_rate();

    event_queue_t & outstanding_events() { return m_outstanding_events; }
  private:
    mutable mutex_type m_stats_mutex;

    // disallow copy construction
    Master(Master const & other);
    Master & operator=(Master const & other);

    std::string m_name;
    state_code m_state;
    time_type m_last_recv;
    time_type m_last_send;
    time_type m_last_job_recv;
    time_type m_last_job_rqst;

    size_t m_num_send;
    size_t m_num_recv;
    size_t m_num_jobs_recv;
    size_t m_num_jobs_rqst;

    bool m_polling;
    size_t        m_poll_backoff_counter;
    time_duration m_min_poll_interval;
    time_duration m_cur_poll_interval;
    time_duration m_max_poll_interval;

    event_queue_t m_outstanding_events;
  };
}

#endif // DRTS_MASTER_HPP
