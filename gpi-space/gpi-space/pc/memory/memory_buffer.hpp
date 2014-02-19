#ifndef GPI_SPACE_PC_MEMORY_BUFFER_HPP
#define GPI_SPACE_PC_MEMORY_BUFFER_HPP

#include <boost/utility.hpp>

#include <unistd.h> // size_t
#include <vector>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class buffer_t : boost::noncopyable
      {
      public:
        explicit
        buffer_t (size_t sz);

        inline char *data ()        { return &m_data[0]; }
        inline size_t size () const { return m_data.size(); }
        inline size_t used () const { return m_used; }
        inline void used (size_t u) { m_used = u; }
      private:
        std::vector<char> m_data;
        size_t m_used;
      };
    }
  }
}

#endif
