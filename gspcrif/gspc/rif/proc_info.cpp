#include "proc_info.hpp"

#include "process.hpp"
#include "buffer.hpp"

namespace gspc
{
  namespace rif
  {
    proc_info_t::proc_info_t ()
      : m_proc (-1)
      , m_pid (-1)
      , m_argv ()
      , m_env ()
      , m_status ()
      , m_inp_pending (false)
      , m_out_pending (false)
      , m_err_pending (false)
    {}

    void proc_info_t::assign_from (process_t const &p)
    {
      m_proc = p.id ();
      m_pid = p.pid ();
      m_argv = p.argv ();
      m_env = p.env ();
      m_status = p.status ();

      m_inp_pending = p.buffer  (STDIN_FILENO).size ();
      m_out_pending = p.buffer (STDOUT_FILENO).size ();
      m_err_pending = p.buffer (STDERR_FILENO).size ();
    }
  }
}
