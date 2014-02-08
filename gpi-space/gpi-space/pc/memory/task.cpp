#include "task.hpp"

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      task_t::task_t ( std::string const nme
                     , function_type fun
                     )
        : m_state (task_state::pending)
        , m_name (nme)
        , m_func (fun)
      {}

      void
      task_t::execute ()
      {
        assert (task_state::pending == get_state());

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
      }

      void task_t::set_state (const task_state::state s)
      {
        lock_type lock (m_mutex);
        m_state = s;
        m_state_changed.notify_all();
      }

      task_state::state task_t::get_state () const
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

      bool
      task_t::has_failed () const
      {
        return task_state::failed == get_state();
      }

      bool
      task_t::USED_IN_TEST_ONLY_has_finished () const
      {
        return task_state::finished == get_state();
      }

      bool task_t::USED_IN_TEST_ONLY_is_pending() const
      {
        return task_state::pending == get_state();
      }


      std::string
      task_t::get_error_message () const
      {
        return boost::diagnostic_information(m_error);
      }
    }
  }
}
