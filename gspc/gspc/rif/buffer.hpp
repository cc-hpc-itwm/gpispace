#ifndef GSPC_RIF_BUFFER_HPP
#define GSPC_RIF_BUFFER_HPP

#include <vector>

#include <boost/utility.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace gspc
{
  namespace rif
  {
    class buffer_t : boost::noncopyable
    {
    public:
      explicit
      buffer_t (size_t bytes);

      ssize_t read  (char *dst, size_t len);
      ssize_t write (const char *src, size_t len);

      ssize_t read_from (int fd);
      ssize_t write_to (int fd);

      size_t size () const;
    private:
      typedef boost::shared_mutex            mutex_type;
      typedef boost::shared_lock<mutex_type> shared_lock;
      typedef boost::unique_lock<mutex_type> unique_lock;

      mutable mutex_type m_mutex;

      std::vector<char> m_data;
    };
  }
}

#endif
