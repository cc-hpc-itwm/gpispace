#include "net.hpp"

#include <unistd.h> // getpid

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include <fhgcom/peer.hpp>
#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/thread/event.hpp>
#include <fhg/util/threadname.hpp>

class PeerImpl : FHG_PLUGIN
               , public net::Peer
{
  typedef boost::mutex mutex_type;
  typedef boost::condition_variable condition_type;
  typedef boost::unique_lock<mutex_type> lock_type;

public:
  FHG_PLUGIN_START()
  {
    m_name =      fhg_kernel()->get ("name", fhg_kernel ()->get_name ());

    // initialize peer
    m_peer.reset (new fhg::com::peer_t ( m_name
                                       , fhg::com::host_t(fhg_kernel()->get("host", "*"))
                                       , fhg::com::port_t(fhg_kernel()->get("port", "0"))
                                       )
                 );
    m_peer_thread.reset(new boost::thread(&fhg::com::peer_t::run, m_peer));
    fhg::util::set_threadname (*m_peer_thread, "[net]");

    try
    {
      m_peer->start();
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not start peer: " << ex.what());
      FHG_PLUGIN_FAILED(EAGAIN);
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    if (m_peer)
    {
      m_peer->stop();
    }

    if (m_peer_thread)
    {
      m_peer_thread->interrupt();
      m_peer_thread->join();
    }

    m_peer_thread.reset();
    m_peer.reset();

    FHG_PLUGIN_STOPPED();
  }

  std::string const & name () const
  {
    return m_name;
  }

  int send (std::string const & to, std::string const & data)
  {
    if (! m_peer)
    {
      return -EFAULT;
    }

    typedef fhg::util::thread::event<boost::system::error_code> async_op_t;
    async_op_t send_finished;

    m_peer->async_send ( to, data
                       , boost::bind (&async_op_t::notify, &send_finished, _1)
                       );

    boost::system::error_code ec;
    send_finished.wait (ec);
    return -ec.value();
  }

  int recv (std::string & from, std::string & data)
  {
    if (! m_peer)
    {
      return -EFAULT;
    }

    typedef fhg::util::thread::event<boost::system::error_code> async_op_t;
    async_op_t recv_finished;

    fhg::com::message_t m;
    m_peer->async_recv ( &m
                       , boost::bind (&async_op_t::notify, &recv_finished, _1)
                       );

    boost::system::error_code ec;
    recv_finished.wait (ec);

    data.assign(m.data.begin(), m.data.end());
    from = m_peer->resolve (m.header.src, "*unknown*");
    return -ec.value();
  }
private:
  std::string m_name;
  boost::shared_ptr<boost::thread>    m_peer_thread;
  boost::shared_ptr<fhg::com::peer_t> m_peer;
};

EXPORT_FHG_PLUGIN( net
                 , PeerImpl
                 , "net"
                 , "provides access to the peer infrastructure"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "kvs"
                 , ""
                 );
