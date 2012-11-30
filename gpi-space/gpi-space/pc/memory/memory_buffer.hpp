#ifndef GPI_SPACE_PC_MEMORY_BUFFER_HPP
#define GPI_SPACE_PC_MEMORY_BUFFER_HPP

#include <unistd.h> // size_t

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class buffer_t
      {
      public:
        explicit
        buffer_t (size_t sz);

        ~buffer_t ();

        inline char *data ()        { return m_data; }
        inline size_t size () const { return m_size; }
        inline size_t used () const { return m_used; }
        inline void used (size_t u) { m_used = u; }
      private:
        char  *m_data;
        size_t m_size;
        size_t m_used;
      };
    }
  }
}

#endif
