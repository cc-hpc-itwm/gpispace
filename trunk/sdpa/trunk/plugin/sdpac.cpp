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
      LOG(ERROR, "dependency \"net\" is not of type net::Peer!");
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
    return -EFAULT;
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
