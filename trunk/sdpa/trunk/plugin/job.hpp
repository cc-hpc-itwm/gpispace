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
      , CANCELLED
      };

    struct ID
    {
      explicit ID(std::string const &s)
        : value(s)
      {}

      const std::string value;
    private:
      ID(ID const &);
      ID& operator=(ID const &);
    };

    struct Description
    {
      explicit Description(std::string const &s)
        : value(s)
      {}

      const std::string value;
    private:
      Description(Description const &);
      Description& operator=(Description const &);
    };

    struct Owner
    {
      explicit Owner(std::string const &s)
        : value(s)
      {}

      const std::string value;
    private:
      Owner(Owner const &);
      Owner& operator=(Owner const &);
    };

    explicit
    Job( Job::ID const &jobid
       , Job::Description const &description
       , Job::Owner const &owner
       );

    ~Job() {}

    inline state_t state () const { lock_type lck(m_mutex); return m_state; }
    inline void    state (state_t s) { lock_type lck(m_mutex); m_state = s; }

    std::string const & id() const { return m_id; }
    std::string const & description() const { return m_description; }
    std::string const & owner() const { return m_owner; }

    std::string const & result() const { lock_type lck(m_mutex); return m_result; }
    void result(std::string const &r) { lock_type lck(m_mutex); m_result = r; }
  private:
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
