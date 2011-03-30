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

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      typedef std::size_t tag_t;

      class topology_t : boost::noncopyable
      {
      public:
        explicit
        topology_t (const gpi::rank_t rank);

        void add_neighbour(const gpi::rank_t rank);
        void del_neighbour(const gpi::rank_t rank);

        void initialize();
        void shutdown();

        /**
         * Cast a single message to a given rank.
         *
         * @param to the rank to cast the message to
         */
        tag_t cast (gpi::rank_t to, ...);

        // signals
        //    alloc-requested(handle_t, offset, size)
        //       -> return: No, Yes, Defrag
        //    defrag-requested()
        //       -> return: nil
        //    free-requested(handle_t)
        //       -> return: No, Yes (just for sanity)
      private:
        typedef boost::mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::shared_ptr<boost::thread> thread_ptr;
        typedef boost::shared_ptr<fhg::com::peer_t> peer_ptr;

        mutable mutex_type m_mutex;
        thread_ptr m_peer_thread;
        peer_ptr   m_peer;
        gpi::rank_t m_rank;
      };
    }
  }
}

#endif // GPI_SPACE_GLOBAL_TOPOLOGY_HPP
