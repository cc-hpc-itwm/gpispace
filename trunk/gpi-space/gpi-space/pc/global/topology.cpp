#include "topology.hpp"

#include <boost/lexical_cast.hpp>

#include <fhglog/minimal.hpp>

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
        : m_rank ((gpi::rank_t)-1)
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
      }

      void topology_t::stop ()
      {
        lock_type lock(m_mutex);
        if (! m_peer_thread)
        {
          return;
        }

        // TODO: disconnect from all neighbors, shutdown the peer
        m_peer->stop();
        m_peer_thread->join();
        m_peer.reset();
        m_peer_thread.reset();
      }

      void topology_t::establish ()
      {
        // TODO: connect to all neighbors
      }
    }
  }
}
