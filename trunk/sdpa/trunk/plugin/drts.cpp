#include "drts.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

//#include <gpi-space/pc/plugin/gpi.hpp>
#include <sdpa/events/Codec.hpp>

#include <fhgcom/peer.hpp>
#include <fhgcom/kvs/kvsc.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

class DRTSImpl : FHG_PLUGIN
               , public drts::DRTS
{
public:
  FHG_PLUGIN_START()
  {
    m_shutting_down = false;

    try
    {
      fhg::com::kvs::global::get_kvs_info().init( fhg_kernel()->get("kvs_host", "localhost")
                                                , fhg_kernel()->get("kvs_port", "2439")
                                                , boost::posix_time::seconds(2)
                                                , 1
                                                );
      fhg::com::kvs::global::get_kvs_info().start();
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not connect to KVS: " << ex.what());
      FHG_PLUGIN_FAILED(ECONNREFUSED);
    }

    // initialize peer
    m_peer.reset (new fhg::com::peer_t ( fhg_kernel()->get("name", "drts")
                                       , fhg::com::host_t(fhg_kernel()->get("host", "node010"))
                                       , fhg::com::port_t(fhg_kernel()->get("port", "0"))
                                       )
                 );
    m_peer_thread.reset(new boost::thread(&fhg::com::peer_t::run, m_peer));
    try
    {
      m_peer->start();
      start_receiver();
    }
    catch (std::exception const &ex)
    {
      LOG(ERROR, "could not start peer: " << ex.what());
      FHG_PLUGIN_FAILED(EAGAIN);
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    LOG(INFO, "stopping peer");
    // cancel running jobs etc.
    m_shutting_down = true;
    if (m_peer)
    {
      m_peer->stop();
    }

    if (m_peer_thread)
    {
      m_peer_thread->interrupt();
      m_peer_thread->join();
    }

    m_peer.reset();
    m_peer_thread.reset();

    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    if ("gpi" == plugin)
    {
      LOG(INFO, "gained capability: gpi");
    }
  }

  FHG_ON_PLUGIN_UNLOAD(plugin)
  {
    if ("gpi" == plugin)
    {
      LOG(INFO, "lost capability: gpi");
    }
  }

  int exec (drts::job_desc_t const &, drts::job_id_t &, drts::JobListener *)
  {
    // parse job
    //   if ok:
    //      assign job id
    //      state == pending
    //      schedule job with attached listener
    //   else:
    //      return -EINVAL
    return -EPERM;
  }

  drts::status::status_t query (drts::job_id_t const & jobid)
  {
    return drts::status::FAILED;
  }

  int cancel (drts::job_id_t const & jobid)
  {
    return -ESRCH;
  }

  int results (drts::job_id_t const & jobid, std::string &)
  {
    return -ESRCH;
  }

  int remove (drts::job_id_t const & jobid)
  {
    return -ESRCH;
  }

  int add_agent (std::string const & name)
  {
    // 1. remember agent in agent map
    //   state - not connected
    // 2. send registration event to agent
    //   re-schedule registration send
    // 3. when we receive an acknowledge
    //   change agents state to 'connected'
    //   no re-schedule of registration
    // 4. on connection lost -> del_agent
    return 0;
  }

  int del_agent (std::string const & name)
  {
    //  change state to 'not-connected'
    //  if there are jobs from that agent:
    //     remember finished jobs (with timestamp)
    //     what should happen with not yet executed jobs?
    //  else: remove agent
    return 0;
  }
private:
  void do_start()
  {
    // start scheduler threads (configured #)
    // initialize gui-observer if requested
  }

  void start_receiver()
  {
    m_peer->async_recv(&m_message, boost::bind( &DRTSImpl::handle_recv
                                              , this
                                              , _1
                                              )
                      );
  }

  void handle_recv (boost::system::error_code const & ec)
  {
    static sdpa::events::Codec codec;

    if (! ec)
    {
      // convert m_message to event
      try
      {
        sdpa::events::SDPAEvent::Ptr evt
          (codec.decode (std::string (m_message.data.begin(), m_message.data.end())));
        DLOG(TRACE, "received event: " << evt->str());
      }
      catch (std::exception const & ex)
      {
        LOG(WARN, "could not handle incoming message: " << ex.what());
      }
      start_receiver();
    }
    else if (! m_shutting_down)
    {
      const fhg::com::p2p::address_t & addr = m_message.header.src;
      if (addr != m_peer->address())
      {
        DLOG(WARN, "connection to " << m_peer->resolve (addr, "*unknown*") << " lost: " << ec);

        sdpa::events::ErrorEvent::Ptr
          error(new sdpa::events::ErrorEvent ( m_peer->resolve(addr, "*unknown*")
                                             , m_peer->name()
                                             , sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                                             , boost::lexical_cast<std::string>(ec)
                                             )
               );
        start_receiver();
      }
      else
      {
        LOG(TRACE, m_peer->name() << " is shutting down");
      }
    }
  }

  bool m_shutting_down;
  boost::shared_ptr<boost::thread>    m_peer_thread;
  boost::shared_ptr<fhg::com::peer_t> m_peer;
  fhg::com::message_t m_message;
  // workflow engine
  // module loader
  // connections to other agents
  // jobs + their states
  // job-queue(s) (1 per core?)
};

EXPORT_FHG_PLUGIN( drts
                 , DRTSImpl
                 , "provides access to the distributed runtime-system"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v.0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
