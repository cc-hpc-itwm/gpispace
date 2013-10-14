#include "response.hpp"

namespace gspc
{
  namespace net
  {
    namespace client
    {
      response_t::response_t (std::string const &id)
        : m_aborted (false)
        , m_reply ()
        , m_mutex ()
        , m_wait_object ()
        , m_id (id)
      {}

      void response_t::wait ()
      {
        boost::unique_lock<boost::mutex> lock (m_mutex);

        while (!m_reply && !m_aborted)
        {
          m_wait_object.wait (lock);
        }
      }

      int response_t::wait (boost::posix_time::time_duration t)
      {
        boost::unique_lock<boost::mutex> lock (m_mutex);

        if (!m_reply && !m_aborted)
        {
          m_wait_object.timed_wait (lock, t);
        }

        if (m_aborted)
          return -ECANCELED;
        if (m_reply)
          return 0;
        return -ETIME;
      }

      void response_t::notify (frame const & f)
      {
        boost::unique_lock<boost::mutex> lock (m_mutex);
        assert (! m_reply);

        m_reply = f;
        m_wait_object.notify_all ();
      }

      void response_t::abort (boost::system::error_code const &ec)
      {
        boost::unique_lock<boost::mutex> lock (m_mutex);
        assert (! m_aborted);

        if (not m_reply)
        {
          m_ec = ec;
          m_aborted = true;
        }
        m_wait_object.notify_all ();
      }

      boost::system::error_code const &response_t::error () const
      {
        boost::unique_lock<boost::mutex> lock (m_mutex);
        return m_ec;
      }

      std::string const &response_t::id () const
      {
        return m_id;
      }

      boost::optional<frame> const & response_t::get_reply () const
      {
        boost::unique_lock<boost::mutex> lock (m_mutex);
        return m_reply;
      }

      bool response_t::is_aborted () const
      {
        boost::unique_lock<boost::mutex> lock (m_mutex);
        return m_aborted;
      }
    }
  }
}
