// bernd.loerwald@itwm.fraunhofer.de

#ifndef GSPC_RIF_DAEMON_HPP
#define GSPC_RIF_DAEMON_HPP

#include <gspc/net/frame.hpp>
#include <gspc/net/server.hpp>
#include <gspc/net/user.hpp>
#include <gspc/rif/manager.hpp>
#include <gspc/rif/supervisor.hpp>

namespace gspc
{
  namespace rif
  {
    class daemon
    {
    public:
      daemon ( boost::function<void()> request_shutdown
             , size_t nthreads, std::string netd_url
             );
      ~daemon();

    private:
      int supervise (std::list<std::string> argv);

      void on_child_failed (gspc::rif::supervisor_t::child_info_t const &chld);
      void on_child_started (gspc::rif::supervisor_t::child_info_t const &chld);
      void on_child_terminated (gspc::rif::supervisor_t::child_info_t const &chld);

      void handle ( std::string const &dst
                  , gspc::net::frame const &rqst
                  , gspc::net::user_ptr user
                  );

      boost::function<void()> _request_shutdown;

      gspc::net::server_ptr_t m_server;
      gspc::rif::manager_t m_mgr;
      gspc::rif::supervisor_t m_supervisor;
    };
  }
}

#endif
