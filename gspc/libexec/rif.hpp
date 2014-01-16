// bernd.loerwald@itwm.fraunhofer.de

#ifndef GSPC_LIBEXEC_RIF_HPP
#define GSPC_LIBEXEC_RIF_HPP

#include <fhg/plugin/plugin.hpp>

#include <gspc/net/frame.hpp>
#include <gspc/net/server.hpp>
#include <gspc/net/user.hpp>
#include <gspc/rif/manager.hpp>
#include <gspc/rif/supervisor.hpp>

class RifImpl : FHG_PLUGIN
{
public:
  RifImpl();
  ~RifImpl();

  FHG_PLUGIN_START();
  FHG_PLUGIN_STOP();

private:
  void shutdown ();

  int supervise (std::list<std::string> argv);

  void on_child_failed (gspc::rif::supervisor_t::child_info_t const &chld);
  void on_child_started (gspc::rif::supervisor_t::child_info_t const &chld);
  void on_child_terminated (gspc::rif::supervisor_t::child_info_t const &chld);

  void handle ( std::string const &dst
              , gspc::net::frame const &rqst
              , gspc::net::user_ptr user
              );

  gspc::net::server_ptr_t m_server;
  gspc::rif::manager_t m_mgr;
  gspc::rif::supervisor_t m_supervisor;
};

#endif
