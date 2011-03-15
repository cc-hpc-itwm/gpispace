#ifndef GPI_SPACE_PC_MEMORY_TASK_HPP
#define GPI_SPACE_PC_MEMORY_TASK_HPP

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace task_state
      {
        enum state {pending, executing, finished, failed, cancelled};
      }

      class task_t : boost::noncopyable
      {
      public:
        typedef task_state::state state;
        typedef boost::function<void (void)> function_type;

        template <typename F>
        task_t (std::string const nme, F fun)
          : m_state (task_state::pending)
          , m_name (nme)
          , m_func (fun)
        {}

        ~task_t ();

        void execute ();
        void cancel ();
        void reset ();
        void wait ();

        std::string const & get_name () const;
        boost::exception_ptr get_error () const;
        std::string get_error_message () const;
        state get_state () const;
        bool has_failed () const;
        bool has_finished () const;
      private:
        typedef boost::mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::condition_variable condition_type;

        void set_state (const state);

        mutable mutex_type m_mutex;
        condition_type m_state_changed;
        state m_state;
        const std::string m_name;
        function_type m_func;
        boost::exception_ptr m_error;
      };
    }
  }
}

#endif
