/**
 * @file   topology.hpp
 * @author Alexander Petry <petry@itwm.fhg.de>
 * @date   Thu Mar 17 10:08:55 2011
 *
 * @brief  Topology between GPI nodes based on fhgcom
 */

#ifndef GPI_SPACE_GLOBAL_TOPOLOGY_HPP
#define GPI_SPACE_GLOBAL_TOPOLOGY_HPP 1

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include <fhgcom/peer.hpp>

#include <gpi-space/types.hpp>
#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle.hpp>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      struct message_t
      {
        struct header_t;
        struct body_t;
      };

      class topology_t : boost::noncopyable
      {
      public:
        typedef fhg::com::port_t port_t;
        typedef fhg::com::host_t host_t;

        static port_t const & any_port ();
        static host_t const & any_addr ();

        explicit
        topology_t ();

        ~topology_t ();

        void add_neighbor(const gpi::rank_t rank);
        void del_neighbor(const gpi::rank_t rank);

        void start( const gpi::rank_t rank
                  , const fhg::com::host_t & host
                  , const fhg::com::port_t & port
                  , std::string const & cookie
                  );
        void stop ();

        void establish ();

        // initiate a global alloc
        int global_alloc ( const gpi::pc::type::handle_t hdl
                         , const gpi::pc::type::size_t
                         );

        // signals
        //    alloc-requested(handle_t, offset, size)
        //       -> return: No, Yes, Defrag
        //    defrag-requested()
        //       -> return: nil
        //    free-requested(handle_t)
        //       -> return: No, Yes (just for sanity)
        //    shutdown-requested()
        //       -> return: nil
      private:
        struct neighbor_t
        {
          neighbor_t ()
            : rank((gpi::rank_t)-1)
            , last_signal(0)
          {}

          neighbor_t (const gpi::rank_t r)
            : rank (r)
            , last_signal (0)
          {}

          gpi::rank_t rank;
          std::string name;
          time_t      last_signal;
        };

        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::shared_ptr<boost::thread> thread_ptr;
        typedef boost::shared_ptr<fhg::com::peer_t> peer_ptr;
        typedef std::map<gpi::rank_t, neighbor_t> neighbor_map_t;

        void message_received (boost::system::error_code const &);
        void handle_message ( const gpi::rank_t rank
                            , const fhg::com::message_t &
                            );
        void handle_error ( const gpi::rank_t rank
                          , boost::system::error_code const &
                          );

        /**
         * Cast a single message to a given rank.
         *
         * @param to the rank to cast the message to
         */
        void cast (const gpi::rank_t rnk, const char *data, const std::size_t len);
        void cast (neighbor_t const &, const char *data, const std::size_t len);

        void broadcast(const char *data, const std::size_t len);

        mutable mutex_type m_mutex;
        bool m_shutting_down;
        gpi::rank_t m_rank;
        thread_ptr m_peer_thread;
        peer_ptr   m_peer;
        neighbor_map_t m_neighbors;
        fhg::com::message_t m_incoming_msg;
      };
    }
  }
}

#endif // GPI_SPACE_GLOBAL_TOPOLOGY_HPP
