#ifndef GSPC_RIF_PIPE_HPP
#define GSPC_RIF_PIPE_HPP

#include <unistd.h>

namespace gspc
{
  namespace rif
  {
    class pipe_t
    {
    public:
      pipe_t ();
      ~pipe_t ();

      int rd () const { return m_fd [0]; }
      int wr () const { return m_fd [1]; }

      ssize_t read (void *buffer, size_t len);
      ssize_t write (const void *buffer, size_t len);

      int open (int flags, bool close_on_exec);
      int close ();
      int close_rd ();
      int close_wr ();
    private:
      int m_fd [2];
    };
  }
}

#endif
