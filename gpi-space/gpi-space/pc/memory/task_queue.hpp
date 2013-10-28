#ifndef GPI_SPACE_PC_MEMORY_TASK_QUEUE_HPP
#define GPI_SPACE_PC_MEMORY_TASK_QUEUE_HPP

#include <deque>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <gpi-space/pc/memory/task.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class task_queue_t : boost::noncopyable
      {
      public:
        typedef boost::shared_ptr<task_t> task_ptr;
        typedef std::deque<task_ptr> container_type;
        typedef container_type::size_type size_type;

        ~task_queue_t();

        void     push(task_ptr);
        task_ptr pop();
        void     cancel();
        void     cancel(task_ptr);

        bool empty() const;
        size_type size() const;
        void clear();

      private:
        typedef boost::mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::condition_variable condition_type;

        mutable mutex_type m_mutex;
        condition_type m_task_available;
        container_type m_tasks;
      };
    }
  }
}

#endif
