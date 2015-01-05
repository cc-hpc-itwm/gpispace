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
#include <boost/thread/condition.hpp>

#include <fhgcom/peer.hpp>

#include <gpi-space/pc/global/itopology.hpp>

#include <gpi-space/types.hpp>
#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle.hpp>
#include <gpi-space/pc/memory/manager.hpp>

#include <functional>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      class topology_t : boost::noncopyable
                       , public itopology_t
      {
      public:
        typedef fhg::com::port_t port_t;
        typedef fhg::com::host_t host_t;

        topology_t ( memory::manager_t& memory_manager
                   , api::gpi_api_t&
                   , boost::shared_ptr<fhg::com::peer_t> const&
                   );
        ~topology_t ();

        virtual bool is_master () const override;

        // initiate a global alloc
        virtual void alloc ( const gpi::pc::type::segment_id_t segment
                           , const gpi::pc::type::handle_t
                           , const gpi::pc::type::offset_t
                           , const gpi::pc::type::size_t size
                           , const gpi::pc::type::size_t local_size
                           , const std::string & name
                           ) override;

        virtual void free (const gpi::pc::type::handle_t) override;

        virtual void add_memory ( const gpi::pc::type::segment_id_t seg_id
                                , const std::string & url
                                ) override;
        virtual void del_memory (const gpi::pc::type::segment_id_t seg_id) override;
      private:
        // signals
        //    alloc-requested(handle_t, offset, size)
        //       -> return: No, Yes, Defrag
        //    defrag-requested()
        //       -> return: nil
        //    free-requested(handle_t)
        //       -> return: No, Yes (just for sanity)
        //    shutdown-requested()
        //       -> return: nil

        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::condition_variable_any condition_type;
        typedef boost::shared_ptr<fhg::com::peer_t> peer_ptr;

        void message_received ( boost::system::error_code const &
                              , boost::optional<fhg::com::p2p::address_t>
                              , memory::manager_t&
                              );
        void message_sent (boost::system::error_code const&);

        void handle_message ( fhg::com::p2p::address_t const&
                            , const std::string &
                            , memory::manager_t&
                            );

        /**
         * Cast a single message to a given rank.
         *
         * @param to the rank to cast the message to
         */
        void cast (fhg::com::p2p::address_t const&, std::string const& data);

        void request (std::string const& name, std::string const& data);

        mutable mutex_type m_mutex;
        mutable mutex_type m_global_alloc_mutex;
        mutable mutex_type m_request_mutex;
        mutable mutex_type m_result_mutex;
        mutable condition_type m_request_finished;

        bool m_shutting_down;
        peer_ptr   m_peer;
        std::unordered_set<fhg::com::p2p::address_t> m_children;
        fhg::com::message_t m_incoming_msg;

        std::vector<boost::optional<std::string>> m_current_results;

        api::gpi_api_t& _gpi_api;
      };
    }
  }
}

#endif // GPI_SPACE_GLOBAL_TOPOLOGY_HPP
