#include "sdpac.hpp"
#include "net.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <sdpa/uuidgen.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/events.hpp>

class SDPACImpl : FHG_PLUGIN
                , public sdpac::SDPAC
{
public:
  FHG_PLUGIN_START()
  {
    m_peer = fhg_kernel()->acquire<net::Peer>("net");
    if (!m_peer)
    {
      LOG(ERROR, "dependency \"net\" is not available or not of type net::Peer!");
      FHG_PLUGIN_FAILED(EINVAL);
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  int submit (std::string const &wf, std::string & job_id)
  {
    // make request
    return -EFAULT;
  }

  int status (std::string const &id)
  {
    sdpa::events::SDPAEvent::Ptr req;
    sdpa::events::SDPAEvent::Ptr rep;

    req.reset (new sdpa::events::QueryJobStatusEvent( m_peer->name()
                                                    , "orchestrator"
                                                    , id
                                                    )
              );
    return request (req, rep);
  }

  int cancel (std::string const &id)
  {
    return -EFAULT;
  }

  int result (std::string const &id, std::string &out)
  {
    return -EFAULT;
  }

  int remove (std::string const &id)
  {
    return -EFAULT;
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
      return -ESRCH;
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
};

EXPORT_FHG_PLUGIN( sdpac
                 , SDPACImpl
                 , "provides the client side interface to the SDPA"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "net"
                 , ""
                 );
