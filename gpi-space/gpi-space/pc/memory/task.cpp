#include "task.hpp"

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      void
      task_t::execute ()
      {
        switch (get_state())
        {
        case task_state::pending:
          try
          {
            set_state (task_state::executing);
            m_func ();
            set_state (task_state::finished);
          }
          catch (...)
          {
            m_error = boost::current_exception();
            set_state (task_state::failed);
          }
          break;
        case task_state::cancelled:
          break;
        case task_state::executing:
          throw std::runtime_error ("task already executing: " + get_name());
          break;
        default:
          throw std::runtime_error ("task already executed: " + get_name());
        }
      }

      void task_t::cancel ()
      {
        if (get_state() == task_state::pending)
        {
          set_state (task_state::cancelled);
        }
      }

      void task_t::reset ()
      {
        set_state (task_state::pending);
      }

      void task_t::set_state (const task_t::state s)
      {
        lock_type lock (m_mutex);
        m_state = s;
        m_state_changed.notify_all();
      }

      task_t::state task_t::get_state () const
      {
        lock_type lock (m_mutex);
        return m_state;
      }

      void
      task_t::wait ()
      {
        lock_type lock (m_mutex);
        while (  task_state::pending   == m_state
              || task_state::executing == m_state
              )
        {
          m_state_changed.wait (lock);
        }
      }

      std::string const &
      task_t::get_name () const
      {
        return m_name;
      }

      boost::exception_ptr
      task_t::get_error () const
      {
        return m_error;
      }

      bool
      task_t::has_failed () const
      {
        return task_state::failed == get_state();
      }

      bool
      task_t::has_finished () const
      {
        return task_state::finished == get_state();
      }

      std::string
      task_t::get_error_message () const
      {
        return boost::diagnostic_information(get_error());
      }

      std::size_t
      task_t::time_estimation () const
      {
        return m_eta;
      }
    }
  }
}
