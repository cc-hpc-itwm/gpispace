#pragma once

#include <boost/noncopyable.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <boost/shared_ptr.hpp>
#include <functional>
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
        enum state {pending, executing, finished, failed};
      }

      class task_t : boost::noncopyable
      {
      public:
        typedef std::function<void (void)> function_type;

        task_t (std::string const name, function_type fun);

        void execute ();
        void wait ();

        void get();

        std::string const & get_name () const;
        std::string get_error_message () const;
        bool has_failed () const;
        bool USED_IN_TEST_ONLY_has_finished () const;
        bool USED_IN_TEST_ONLY_is_pending() const;
      private:
        void set_state (const task_state::state);
        task_state::state get_state () const;

        mutable boost::mutex _mutex_state;
        boost::condition_variable m_state_changed;
        task_state::state m_state;
        const std::string m_name;
        function_type m_func;
        boost::exception_ptr m_error;
      };

      typedef boost::shared_ptr<task_t> task_ptr;
      typedef std::set<task_ptr>        task_set_t;
      typedef std::list<task_ptr>       task_list_t;
    }
  }
}
