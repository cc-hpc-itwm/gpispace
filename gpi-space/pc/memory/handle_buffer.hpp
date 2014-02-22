#ifndef GPI_SPACE_PC_HANDLE_BUFFER_HPP
#define GPI_SPACE_PC_HANDLE_BUFFER_HPP

#include <unistd.h> // size_t

#include <gpi-space/pc/type/handle.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class handle_buffer_t
      {
      public:
        explicit
        handle_buffer_t ( gpi::pc::type::handle_t hdl
                        , size_t sz
                        , void *ptr
                        )
          : m_handle (hdl)
          , m_data (ptr)
          , m_size (sz)
          , m_used (0)
        {}

        inline void *data ()        { return m_data; }
        inline size_t size () const { return m_size; }
        inline size_t used () const { return m_used; }
        inline void used (size_t u) { m_used = u; }

        gpi::pc::type::handle_t handle () const { return m_handle; }
      private:
        gpi::pc::type::handle_t  m_handle;
        void                    *m_data;
        size_t                   m_size;
        size_t                   m_used;
      };
    }
  }
}

#endif
