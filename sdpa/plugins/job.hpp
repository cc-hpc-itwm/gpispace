#ifndef DRTS_PLUGIN_JOB_HPP
#define DRTS_PLUGIN_JOB_HPP 1

#include <list>
#include <string>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>

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

    inline state_t state () const { lock_type lck(m_mutex); return m_state; }
    state_t cmp_and_swp_state( state_t expected
                             , state_t newstate
                             );
    inline Job& set_state (state_t s) {
      lock_type    lck(m_mutex); m_state = s;
      return *this;
    }

    std::string const & id() const { return m_id; }
    std::string const & description() const { return m_input_description; }
    std::string const & owner() const { return m_owner; }

    std::string const & result() const {
      lock_type lck(m_mutex); return m_result;
    }
    Job & set_result(std::string const &r) {
      lock_type lck(m_mutex); m_result = r;
      return *this;
    }

    int result_code () const {
      lock_type lck(m_mutex); return m_result_code;
    }

    Job& set_result_code (const int rc) {
      lock_type lck(m_mutex); m_result_code = rc;
      return *this;
    }

    std::string const & message() const {
      lock_type lck(m_mutex); return m_message;
    }

    Job& set_message (std::string const &s) {
      lock_type lck(m_mutex); m_message = s;
      return *this;
    }

    boost::posix_time::ptime const & entered () const { return m_entered; }
    boost::posix_time::ptime const & started () const { return m_started; }
    boost::posix_time::ptime const & completed () const { return m_completed; }

    Job& entered   (boost::posix_time::ptime const &t) {
      m_entered = t;
      return *this;
    }
    Job& started   (boost::posix_time::ptime const &t) {
      m_started = t;
      return *this;
    }
    Job& completed (boost::posix_time::ptime const &t) {
      m_completed = t;
      return *this;
    }

    std::list<std::string> const &worker_list () const
    {
      return m_worker_list;
    }

    Job & worker_list (std::list<std::string> const &workers)
    {
      m_worker_list = workers;
      return *this;
    }
  private:
    Job () {}

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize (Archive & ar, const unsigned int version)
    {
      ar & BOOST_SERIALIZATION_NVP(m_id);
      ar & BOOST_SERIALIZATION_NVP(m_input_description);
      ar & BOOST_SERIALIZATION_NVP(m_owner);
      ar & BOOST_SERIALIZATION_NVP(m_state);
      ar & BOOST_SERIALIZATION_NVP(m_result);

      if (version > 0)
      {
        ar & BOOST_SERIALIZATION_NVP(m_entered);
        ar & BOOST_SERIALIZATION_NVP(m_started);
        ar & BOOST_SERIALIZATION_NVP(m_completed);
      }

      if (version > 1)
      {
        ar & BOOST_SERIALIZATION_NVP(m_result_code);
        ar & BOOST_SERIALIZATION_NVP(m_message);
      }

      if (version > 2)
      {
        ar & BOOST_SERIALIZATION_NVP(m_worker_list);
      }
    }

    inline void    state (state_t s) { lock_type lck(m_mutex); m_state = s; }
    mutable mutex_type m_mutex;

    std::string m_id;
    std::string m_input_description;
    std::string m_owner;
    state_t     m_state;
    std::string m_result;
    int         m_result_code;
    std::string m_message;
    std::list<std::string> m_worker_list;

    // timestamps
    boost::posix_time::ptime m_entered;
    boost::posix_time::ptime m_started;
    boost::posix_time::ptime m_completed;
  };
}

BOOST_CLASS_VERSION(drts::Job, 3);

#endif
