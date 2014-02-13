/**
 * @file   topology.hpp
 * @author Alexander Petry <petry@itwm.fhg.de>
 * @date   Thu Mar 17 10:08:55 2011
 *
 * @brief  Topology between GPI nodes based on fhgcom
 */

#ifndef GPI_SPACE_GLOBAL_TOPOLOGY_HPP
#define GPI_SPACE_GLOBAL_TOPOLOGY_HPP 1

#include <boost/function.hpp>
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

        struct rank_result_t
        {
          rank_result_t ()
            : rank(-1)
            , value(-1)
            , message ("")
          {}

          rank_result_t (gpi::rank_t r, int v, std::string const &msg="")
            : rank(r)
            , value(v)
            , message (msg)
          {}

          gpi::rank_t rank;
          int value;
          std::string message;
        };

        typedef boost::function<rank_result_t ( rank_result_t
                                              , rank_result_t
                                              )
                               > fold_t;

        static port_t const & any_port ();
        static host_t const & any_addr ();

        topology_t ( const gpi::rank_t rank
                   , const fhg::com::host_t & host
                   , const fhg::com::port_t & port
                   , std::string const & cookie
                   , memory::manager_t& memory_manager
                   );
        ~topology_t ();

        void add_child(const gpi::rank_t rank);
        void del_child(const gpi::rank_t rank);

        void establish ();

        bool is_master () const;

        int go ();
        int wait_for_go ();

        // initiate a global alloc
        int alloc ( const gpi::pc::type::segment_id_t segment
                  , const gpi::pc::type::handle_t
                  , const gpi::pc::type::offset_t
                  , const gpi::pc::type::size_t size
                  , const gpi::pc::type::size_t local_size
                  , const std::string & name
                  );

        int free (const gpi::pc::type::handle_t);

        int add_memory ( const gpi::pc::type::segment_id_t seg_id
                       , const std::string & url
                       );
        int del_memory (const gpi::pc::type::segment_id_t seg_id);
      private:
        void cast (const gpi::rank_t rnk, const std::string & data);
        void cast (const gpi::rank_t rnk, const char *data, const std::size_t len);

        void broadcast(const std::string & data);
        void broadcast(const char *data, const std::size_t len);

        rank_result_t
        all_reduce ( std::string const & req
                   , fold_t fold_fun
                   , rank_result_t myresult
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

        struct child_t
        {
          child_t ()
            : rank((gpi::rank_t)-1)
            , last_signal(0)
            , error_counter(0)
          {}

          child_t (const gpi::rank_t r)
            : rank (r)
            , last_signal (0)
            , error_counter (0)
          {}

          gpi::rank_t rank;
          std::string name;
          time_t      last_signal;
          std::size_t error_counter;
        };

        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::condition_variable_any condition_type;
        typedef boost::shared_ptr<boost::thread> thread_ptr;
        typedef boost::shared_ptr<fhg::com::peer_t> peer_ptr;
        typedef std::map<gpi::rank_t, child_t> child_map_t;
        typedef std::list<rank_result_t> result_list_t;

        void message_received ( boost::system::error_code const &
                              , memory::manager_t&
                              );
        void message_sent ( child_t & child
                          , std::string const & data
                          , boost::system::error_code const &
                          );

        void handle_message ( const gpi::rank_t rank
                            , const std::string &
                            , memory::manager_t&
                            );
        void handle_error ( const gpi::rank_t rank
                          , boost::system::error_code const &
                          );

        /**
         * Cast a single message to a given rank.
         *
         * @param to the rank to cast the message to
         */
        void cast (child_t const &, const std::string &data);

        result_list_t request (const std::string &data);

        mutable mutex_type m_mutex;
        mutable mutex_type m_global_alloc_mutex;
        mutable mutex_type m_request_mutex;
        mutable mutex_type m_result_mutex;
        mutable condition_type m_request_finished;
        mutable mutex_type m_go_event_mutex;
        mutable condition_type m_go_received_event;

        bool m_shutting_down;
        bool m_go_received;
        size_t m_waiting_for_go;
        bool m_established;
        gpi::rank_t m_rank;
        fhg::com::kvs::kvsc_ptr_t _kvs_client;
        peer_ptr   m_peer;
        thread_ptr m_peer_thread;
        child_map_t m_children;
        fhg::com::message_t m_incoming_msg;

        result_list_t m_current_results;
      };
    }
  }
}

#endif // GPI_SPACE_GLOBAL_TOPOLOGY_HPP
