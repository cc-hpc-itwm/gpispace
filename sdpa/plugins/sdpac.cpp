#include "sdpac.hpp"
#include "net.hpp"
#include "kvs.hpp"

#include <errno.h>
#include <unistd.h>

#include <fhglog/LogMacros.hpp>
#include <fhg/plugin/plugin.hpp>

#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/events.hpp>

#include <gspc/net/client.hpp>
#include <gspc/net/dial.hpp>
#include <gspc/net/frame.hpp>

class SDPACImpl : FHG_PLUGIN
                , public sdpa::Client
{
public:
  FHG_PLUGIN_START()
  {
    m_orchestrator = fhg_kernel()->get("orchestrator", "orchestrator");
    m_peer = fhg_kernel()->acquire<net::Peer>("net");
    if (!m_peer)
    {
      MLOG(ERROR, "dependency \"net\" is not available or not of type net::Peer!");
      FHG_PLUGIN_FAILED (ELIBACC);
    }
    m_kvs = fhg_kernel ()->acquire<kvs::KeyValueStore>("kvs");
    if (not m_kvs)
    {
      MLOG(ERROR, "dependency \"net\" is not available or not of type net::Peer!");
      FHG_PLUGIN_FAILED (ELIBACC);
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  int execute (std::string const &wf, std::string & out)
  {
    int ec = 0;
    std::string id;
    ec = submit(wf, id);
    if (ec != 0) return ec;

    int state (0);
    for (;;)
    {
      int rc; std::string err;

      state = status (id, rc, err);

      if (state < 0)
      {
        break;
      }
      else if (state < sdpa::status::FINISHED)
      {
        usleep (250);
      }
      else
      {
        break;
      }
    }
    result (id, out);
    remove (id);
    return state;
  }

  int submit (std::string const &wf, std::string & job_id)
  {
    using namespace sdpa::events;

    SDPAEvent::Ptr req;
    SDPAEvent::Ptr rep;

    req.reset (new SubmitJobEvent( m_peer->name()
                                 , m_orchestrator
                                 , sdpa::job_id_t::invalid_job_id() // job id not known
                                 , wf
                                 , sdpa::job_id_t::invalid_job_id() // parent job id
                                 )
              );
    if (request(req, rep) == 0)
    {
      if (SubmitJobAckEvent* ack = dynamic_cast<SubmitJobAckEvent*>(rep.get()))
      {
        job_id = ack->job_id();
        return 0;
      }
      else if (ErrorEvent* error = dynamic_cast<ErrorEvent*>(rep.get()))
      {
        MLOG( WARN
            , "could not query job: " << error->reason() << ": " << error->error_code()
            );
        return -EINVAL;
      }
    }

    return -EIO;
  }

  int status (std::string const &id, int & ec, std::string & msg)
  {
    using namespace sdpa::events;

    SDPAEvent::Ptr req;
    SDPAEvent::Ptr rep;

    req.reset (new QueryJobStatusEvent( m_peer->name()
                                      , m_orchestrator
                                      , id
                                      )
              );
    if (request(req, rep) == 0)
    {
      if (JobStatusReplyEvent* job_status = dynamic_cast<JobStatusReplyEvent*>(rep.get()))
      {
        ec = job_status->error_code ();
        msg = job_status->error_message ();
        return job_status->status();
      }
      else if (ErrorEvent* error = dynamic_cast<ErrorEvent*>(rep.get()))
      {
        MLOG( WARN
            , "could not query job: " << error->reason() << ": " << error->error_code()
            );
        return -ESRCH;
      }
    }

    return -EIO;
  }

  int cancel (std::string const &id)
  {
    using namespace sdpa::events;

    SDPAEvent::Ptr req;
    SDPAEvent::Ptr rep;

    req.reset (new CancelJobEvent (m_peer->name(), m_orchestrator, id));
    if (request(req, rep) == 0)
    {
      if (dynamic_cast<CancelJobAckEvent*>(rep.get()))
      {
        return 0;
      }
      else if (ErrorEvent* error = dynamic_cast<ErrorEvent*>(rep.get()))
      {
        MLOG( WARN
            , "could not cancel job: " << error->reason() << ": " << error->error_code()
            );
        return -ESRCH;
      }
    }

    return -EFAULT;
  }

  int result (std::string const &id, std::string &out)
  {
    using namespace sdpa::events;

    SDPAEvent::Ptr req;
    SDPAEvent::Ptr rep;

    req.reset (new RetrieveJobResultsEvent( m_peer->name()
                                          , m_orchestrator
                                          , id
                                          )
              );
    if (request(req, rep) == 0)
    {
      if (JobResultsReplyEvent* ack = dynamic_cast<JobResultsReplyEvent*>(rep.get()))
      {
        out = ack->result();
        return 0;
      }
      else if (ErrorEvent* error = dynamic_cast<ErrorEvent*>(rep.get()))
      {
        MLOG( WARN
            , "could not get results: " << error->reason() << ": " << error->error_code()
            );
        return -ESRCH;
      }
    }

    return -EFAULT;
  }

  int remove (std::string const &id)
  {
    using namespace sdpa::events;

    SDPAEvent::Ptr req;
    SDPAEvent::Ptr rep;

    req.reset (new DeleteJobEvent( m_peer->name()
                                 , m_orchestrator
                                 , id
                                 )
              );
    if (request(req, rep) == 0)
    {
      if (dynamic_cast<DeleteJobAckEvent*>(rep.get()))
      {
        return 0;
      }
      else if (ErrorEvent* error = dynamic_cast<ErrorEvent*>(rep.get()))
      {
        MLOG( WARN
            , "could not delete job: " << error->reason() << ": " << error->error_code()
            );
        return -ESRCH;
      }
    }

    return -EFAULT;
  }

  int unload_modules ()
  {
    typedef kvs::KeyValueStore::key_value_map_type kv_map_t;
    kv_map_t entries = m_kvs->list ("gspc.net.url.");

    BOOST_FOREACH (kv_map_t::value_type const &kv, entries)
    {
      const std::string url = kv.second;

      try
      {
        gspc::net::client_ptr_t client = gspc::net::dial (url);

        gspc::net::frame rqst ("SEND");
        rqst.set_header ("destination", "/service/wfe/unload-modules");

        gspc::net::frame rply;
        client->request ( rqst
                        , rply
                        , boost::posix_time::milliseconds (2000)
                        );
      }
      catch (std::exception const &)
      {
        // ignore
      }
    }

    return 0;
  }
private:
  int request ( sdpa::events::SDPAEvent::Ptr const &req
              , sdpa::events::SDPAEvent::Ptr & rep
              )
  {
    int rc = send_event (req);
    if (0 == rc)
    {
      return recv_event (rep);
    }
    else
    {
      return rc;
    }
  }

  int send_event (sdpa::events::SDPAEvent::Ptr const & evt)
  {
    static sdpa::events::Codec codec;
    int ec = m_peer->send ( evt->to()
                          , codec.encode(evt.get())
                          );

    if (ec)
    {
      MLOG(WARN, "could not send " << evt->str() << " to " << evt->to());
      return -EIO;
    }

    return 0;
  }

  int recv_event (sdpa::events::SDPAEvent::Ptr & evt)
  {
    static sdpa::events::Codec codec;

    std::string from;
    std::string data;

    int ec = -EAGAIN;

    do
    {
      ec = m_peer->recv (from, data);
      if (ec)
      {
        evt.reset
          (new sdpa::events::ErrorEvent ( from
                                        , m_peer->name()
                                        , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                                        , strerror(-ec)
                                        )
          );
        ec = 0;
      }
      else
      {
        try
        {
          evt.reset (codec.decode(data));
          ec = 0;
        }
        catch (std::exception const &ex)
        {
          LOG(ERROR, "invalid message from: " << from << ": " << ex.what());
          ec = -EAGAIN;
        }
      }
    } while (ec == -EAGAIN);

    return 0;
  }

  net::Peer *m_peer;
  kvs::KeyValueStore *m_kvs;
  std::string m_orchestrator;
};

EXPORT_FHG_PLUGIN( sdpac
                 , SDPACImpl
                 , "sdpac"
                 , "provides the client side interface to the SDPA"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "kvs,net"
                 , ""
                 );
