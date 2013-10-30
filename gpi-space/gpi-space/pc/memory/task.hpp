#ifndef GPI_SPACE_PC_MEMORY_TASK_HPP
#define GPI_SPACE_PC_MEMORY_TASK_HPP

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <boost/shared_ptr.hpp>
#include <set>
#include <list>

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
        task_t (std::string const nme, F fun, const std::size_t eta = 0)
          : m_state (task_state::pending)
          , m_name (nme)
          , m_func (fun)
          , m_eta (eta)
        {}

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
        std::size_t time_estimation () const;
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
        std::size_t m_eta;
      };

      typedef boost::shared_ptr<task_t> task_ptr;
      typedef std::set<task_ptr>        task_set_t;
      typedef std::list<task_ptr>       task_list_t;
    }
  }
}

#endif
