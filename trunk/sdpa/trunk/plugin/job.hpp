#ifndef DRTS_PLUGIN_JOB_HPP
#define DRTS_PLUGIN_JOB_HPP 1

#include <string>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace drts
{
  class Job
  {
    typedef boost::mutex mutex_type;
    typedef boost::condition_variable condition_type;
    typedef boost::unique_lock<mutex_type> lock_type;
  public:
    enum state_t
      {
        PENDING = 0
      , RUNNING
      , FINISHED
      , FAILED
      , CANCELED
      };

    struct ID
    {
      explicit ID(std::string const &s)
        : value(s)
      {}

      const std::string value;
    };

    struct Description
    {
      explicit Description(std::string const &s)
        : value(s)
      {}

      const std::string value;
    };

    struct Owner
    {
      explicit Owner(std::string const &s)
        : value(s)
      {}

      const std::string value;
    };

    explicit
    Job( Job::ID const &jobid
       , Job::Description const &description
       , Job::Owner const &owner
       );

    ~Job() {}

    inline state_t state () const { lock_type lck(m_mutex); return m_state; }
    state_t cmp_and_swp_state( state_t expected
                             , state_t newstate
                             );

    std::string const & id() const { return m_id; }
    std::string const & description() const { return m_description; }
    std::string const & owner() const { return m_owner; }

    std::string const & result() const { lock_type lck(m_mutex); return m_result; }
    void result(std::string const &r) { lock_type lck(m_mutex); m_result = r; }

    boost::posix_time::ptime const & entered () const { return m_entered; }
    boost::posix_time::ptime const & started () const { return m_started; }
    boost::posix_time::ptime const & completed () const { return m_completed; }

    void entered   (boost::posix_time::ptime const &t) { m_entered = t; }
    void started   (boost::posix_time::ptime const &t) { m_started = t; }
    void completed (boost::posix_time::ptime const &t) { m_completed = t; }
  private:
    inline void    state (state_t s) { lock_type lck(m_mutex); m_state = s; }
    mutable mutex_type m_mutex;

    std::string m_id;
    std::string m_description;
    std::string m_owner;
    state_t m_state;
    std::string m_result;

    // timestamps
    boost::posix_time::ptime m_entered;
    boost::posix_time::ptime m_started;
    boost::posix_time::ptime m_completed;
  };
}

#endif
