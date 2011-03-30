#include "topology.hpp"

#include <stdio.h>

#include <boost/lexical_cast.hpp>
#include <fhglog/minimal.hpp>

#include <gpi-space/signal_handler.hpp>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      namespace detail
      {
        static std::string rank_to_name (const gpi::rank_t rnk)
        {
          if (rnk == (gpi::rank_t)-1)
          {
            throw std::invalid_argument("invalid rank: -1");
          }

          return "gpi-" + boost::lexical_cast<std::string>(rnk);
        }

        static gpi::rank_t name_to_rank(const std::string &name)
        {
          unsigned int rnk (-1);
          if (sscanf(name.c_str(), "gpi-%u", &rnk) < 1)
          {
            throw std::invalid_argument("invalid name: " + name);
          }
          else
          {
            return rnk;
          }
        }
      }

      fhg::com::port_t const &
      topology_t::any_port ()
      {
        static fhg::com::port_t p("0");
        return p;
      }

      fhg::com::host_t const &
      topology_t::any_addr ()
      {
        static fhg::com::host_t h(boost::asio::ip::host_name());
        return h;
      }

      topology_t::topology_t()
        : m_shutting_down (false)
        , m_rank ((gpi::rank_t)-1)
      {
      }

      topology_t::~topology_t()
      {
        try
        {
          stop ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not stop topology: " << ex.what());
        }
      }

      void topology_t::add_neighbor(const gpi::rank_t rank)
      {
        lock_type lock(m_mutex);

        assert (m_peer);

        neighbor_t new_neighbor(rank);
        new_neighbor.name = detail::rank_to_name (rank);
        m_neighbors[rank] = new_neighbor;

        m_peer->send (new_neighbor.name, "ADD_NEIGHBOR");
      }

      void topology_t::del_neighbor(const gpi::rank_t rank)
      {
        lock_type lock(m_mutex);

        // if connected:
        //    send disconnect to neighbor
        //    remove connection
        m_neighbors.erase (rank);
      }

      void topology_t::start( const gpi::rank_t rank
                            , const fhg::com::host_t & host
                            , const fhg::com::port_t & port
                            , std::string const & cookie
                            )
      {
        lock_type lock(m_mutex);
        if (m_peer_thread)
        {
          LOG(WARN, "topology still running, please stop it first!");
          return;
        }
        m_shutting_down = false;
        m_rank = rank;

        m_peer.reset
          (new fhg::com::peer_t( detail::rank_to_name (m_rank)
                               , host
                               , port
                               , cookie
                               )
          );
        m_peer_thread.reset
          (new boost::thread(boost::bind( &fhg::com::peer_t::run
                                        , m_peer
                                        )
                            )
          );

        try
        {
          m_peer->start ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not start peer: " << ex.what());
          m_peer_thread->interrupt();
          m_peer_thread->join();
          m_peer.reset();
          m_peer_thread.reset();
          throw;
        }

        // start the message handler
        m_peer->async_recv ( &m_incoming_msg
                           , boost::bind( &topology_t::message_received
                                        , this
                                        , _1
                                        )
                           );
      }

      void topology_t::stop ()
      {
        lock_type lock(m_mutex);
        if (! m_peer_thread)
        {
          return;
        }

        // TODO: disconnect from all neighbors, shutdown the peer
        m_shutting_down = true;
        m_peer->stop();
        m_peer_thread->join();
        m_peer.reset();
        m_peer_thread.reset();
      }

      void topology_t::establish ()
      {
        // TODO: connect to all neighbors
      }

      void topology_t::message_received(boost::system::error_code const &ec)
      {
        if (! ec)
        {
          const fhg::com::p2p::address_t & addr = m_incoming_msg.header.src;
          const std::string name(m_peer->resolve(addr, "*unknown*"));

          handle_message (detail::name_to_rank(name), m_incoming_msg);

          m_peer->async_recv ( &m_incoming_msg
                             , boost::bind( &topology_t::message_received
                                          , this
                                          , _1
                                          )
                             );
        }
        else if (! m_shutting_down)
        {
          const fhg::com::p2p::address_t & addr = m_incoming_msg.header.src;
          if (addr != m_peer->address())
          {
            const std::string name(m_peer->resolve(addr, "*unknown*"));

            handle_error (detail::name_to_rank(name), ec);

            m_peer->async_recv ( &m_incoming_msg
                               , boost::bind( &topology_t::message_received
                                            , this
                                            , _1
                                            )
                               );
          }
        }
        else
        {
          LOG(TRACE, "topology on " << m_rank << " is shutting down");
        }
      }

      void topology_t::handle_message ( const gpi::rank_t rank
                                      , fhg::com::message_t const &msg
                                      )
      {
        LOG(INFO, "got data from gpi-" << rank << ": " << msg.buf());
      }

      void topology_t::handle_error ( const gpi::rank_t rank
                                    , boost::system::error_code const &ec
                                    )
      {
        LOG(WARN, "error on connection to gpi node " << rank);
        LOG(ERROR, "node-failover is not available yet, I have to commit seppuku...");
        gpi::signal::handler().raise(15);
      }
    }
  }
}
