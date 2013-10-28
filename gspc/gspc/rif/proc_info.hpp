#ifndef GSPC_RIF_PROC_INFO_HPP
#define GSPC_RIF_PROC_INFO_HPP

#include <boost/optional.hpp>
#include <gspc/rif/types.hpp>

namespace gspc
{
  namespace rif
  {
    class process_t;

    class proc_info_t
    {
    public:
      proc_info_t ();

      pid_t pid () const { return m_pid; }
      proc_t id () const { return m_proc; }
      argv_t const & argv () const { return m_argv; }
      env_t const & env () const { return m_env; }
      boost::optional<int> status () const { return m_status; }

      size_t inp_pending () const { return m_inp_pending; }
      size_t out_pending () const { return m_out_pending; }
      size_t err_pending () const { return m_err_pending; }

      void assign_from (process_t const &);
    private:
      proc_t m_proc;
      pid_t  m_pid;
      argv_t m_argv;
      env_t  m_env;
      boost::optional<int> m_status;

      size_t m_inp_pending;
      size_t m_out_pending;
      size_t m_err_pending;
    };
  }
}

#endif
