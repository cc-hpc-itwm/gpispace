#include "topology.hpp"

#include <unistd.h>
#include <stdio.h>
#include <csignal> // kill

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/assert.hpp>

#include <fhgcom/kvs/kvsc.hpp>

#include <gpi-space/gpi/api.hpp>
#include <gpi-space/pc/memory/manager.hpp>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      namespace detail
      {
        static void kvs_error_handler (boost::system::error_code const &)
        {
          MLOG (ERROR, "could not contact KVS, terminating");
          kill (getpid (), SIGTERM);
        }

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

        struct command_t
        {
          typedef std::vector<std::string> string_vec;

          explicit
          command_t(const char *name)
          {
            parts.push_back(name);
          }

          command_t & operator<< (std::string const &s)
          {
            parts.push_back("\"" + s + "\"");
            return *this;
          }

          template <typename T>
          command_t & operator<< (T const &t)
          {
            parts.push_back (boost::lexical_cast<std::string>(t));
            return *this;
          }

          operator std::string () const
          {
            std::ostringstream osstr;
            for ( std::vector<std::string>::const_iterator it(parts.begin())
                ; it != parts.end()
                ; ++it
                )
            {
              if (it != parts.begin())
                osstr << " ";
              osstr << *it;
            }

            return osstr.str();
          }
        private:
          std::vector<std::string> parts;
        };
      }

      namespace reduce
      {
        topology_t::rank_result_t
        max_result ( const topology_t::rank_result_t a
                   , const topology_t::rank_result_t b
                   )
        {
          return (a.value > b.value ? a : b);
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
        static fhg::com::host_t h ("*");
        return h;
      }

      topology_t::topology_t()
        : m_shutting_down (false)
        , m_go_received (false)
        , m_waiting_for_go (0)
        , m_established (false)
        , m_rank ((gpi::rank_t)-1)
      {}

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

      bool topology_t::is_master () const
      {
        return 0 == m_rank;
      }

      int topology_t::wait_for_go ()
      {
        lock_type lock (m_go_event_mutex);

        ++m_waiting_for_go;

        boost::system_time const timeout
          (boost::get_system_time()+boost::posix_time::seconds(30));

        while (not m_go_received)
        {
          if (m_shutting_down)
            break;
          if (not m_go_received_event.timed_wait (lock, timeout))
          {
            if (not m_go_received)
            {
              --m_waiting_for_go;
              throw std::runtime_error ("did not receive GO within 30 seconds");
            }
          }
        }

        --m_waiting_for_go;
        if (0 == m_waiting_for_go)
        {
          m_go_received = false;
        }

        if (m_shutting_down)
          throw std::runtime_error ("shutting down");

        return 0;
      }

      int topology_t::go ()
      {
        broadcast (detail::command_t("GO"));
        return 0;
      }

      void topology_t::add_child(const gpi::rank_t rank)
      {
        child_t new_child(rank);
        new_child.name = detail::rank_to_name (rank);

        lock_type lock(m_mutex);
        assert (m_peer);
        m_children[rank] = new_child;
      }

      void topology_t::del_child(const gpi::rank_t rank)
      {
        lock_type lock(m_mutex);

        // if connected:
        //    send disconnect to child
        //    remove connection
        m_children.erase (rank);
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

        if (m_established)
        {
          LOG(WARN, "topology seems to be established already!");
        }

        m_shutting_down = false;
        m_rank = rank;
        m_established = false;

        m_peer.reset
          (new fhg::com::peer_t( detail::rank_to_name (m_rank)
                               , host
                               , port
                               , cookie
                               )
          );
        m_peer->set_kvs_error_handler (detail::kvs_error_handler);

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

      int topology_t::free (const gpi::pc::type::handle_t hdl)
      {
        broadcast (detail::command_t("FREE") << hdl);
        return 0;
      }

      topology_t::result_list_t
      topology_t::request (std::string const & req)
      {
        lock_type request_lock (m_request_mutex); // one request

        lock_type result_list_lock (m_result_mutex);
        m_current_results.clear ();

        broadcast (req);

        boost::system_time const timeout
          (boost::get_system_time()+boost::posix_time::seconds(30));
        while (m_current_results.size () != m_children.size())
        {
          if (!m_request_finished.timed_wait (result_list_lock, timeout))
          {
            if (m_current_results.size() == m_children.size())
            {
              break;
            }
            else
            {
              throw std::runtime_error ("request " + req + " timedout after 30 seconds!");
            }
          }
        }

        return m_current_results;
      }

      topology_t::rank_result_t
      topology_t::all_reduce ( std::string const & req
                             , fold_t fold_fun
                             , rank_result_t result
                             )
      {
        result_list_t results (request(req));
        while (results.size())
        {
          result = fold_fun(results.front(), result);
          results.pop_front();
        }
        return result;
      }

      int topology_t::alloc ( const gpi::pc::type::segment_id_t seg
                            , const gpi::pc::type::handle_t hdl
                            , const gpi::pc::type::offset_t offset
                            , const gpi::pc::type::size_t size
                            , const gpi::pc::type::size_t local_size
                            , const std::string & name
                            )
      {
        // lock, so that no other process can make a global alloc
        lock_type alloc_lock(m_global_alloc_mutex);

        // acquire cluster wide access to the gpi resource
        boost::unique_lock<gpi::api::gpi_api_t>
          gpi_lock(gpi::api::gpi_api_t::get());

        try
        {
          rank_result_t res (all_reduce(  detail::command_t("ALLOC")
                                       << seg
                                       << hdl
                                       << offset
                                       << size
                                       << local_size
                                       << name
                                       , reduce::max_result
                                       , rank_result_t (m_rank, 0) // my result
                                       )
                            );
          if (res.value != 0)
          {
            LOG(ERROR,"allocation on node " << res.rank << " failed: " << res.value);
            throw std::runtime_error
              ( "global allocation failed on at least one node: rank "
              + boost::lexical_cast<std::string>(res.rank)
              + " says: "
              + res.message
              );
          }
          else
          {
            return 0;
          }
        }
        catch (std::exception const & ex)
        {
          free (hdl);
          throw;
        }
      }

      int topology_t::add_memory ( const gpi::pc::type::segment_id_t seg_id
                                 , const std::string & url
                                 )
      {
        int rc = 0;

        rank_result_t res (all_reduce(  detail::command_t("ADDMEM")
                                     << seg_id
                                     << url
                                     , reduce::max_result
                                     , rank_result_t (m_rank, 0) // my result
                                     )
                          );
        rc = res.value;

        if (rc != 0)
        {
          LOG ( ERROR
              , "add_memory: failed on node " << res.rank
              << ": " << res.value
              << ": " << res.message
              );
          throw std::runtime_error
            ( "add_memory: failed on at least one node: rank "
            + boost::lexical_cast<std::string>(res.rank)
            + " says: "
            + res.message
            );
        }

        return rc;
      }

      int topology_t::del_memory (const gpi::pc::type::segment_id_t seg_id)
      {
        int rc = 0;

        rank_result_t res (all_reduce(  detail::command_t("DELMEM")
                                     << seg_id
                                     , reduce::max_result
                                     , rank_result_t (m_rank, 0) // my result
                                     )
                          );
        rc = res.value;

        if (rc != 0)
        {
          LOG ( ERROR
              , "del_memory: failed on node " << res.rank
              << ": " << res.value
              << ": " << res.message
              );
          throw std::runtime_error
            ( "del_memory: failed on at least one node: rank "
            + boost::lexical_cast<std::string>(res.rank)
            + " says: "
            + res.message
            );
        }

        return rc;
      }

      void topology_t::stop ()
      {
        {
          lock_type lock(m_mutex);
          if (! m_peer_thread || m_shutting_down)
          {
            return;
          }
          m_shutting_down = true;
        }

        m_peer->stop();
        m_peer_thread->join();
        m_peer.reset();
        m_peer_thread.reset();
      }

      void topology_t::establish ()
      {
        LOG(TRACE, "establishing topology...");

        try
        {
          rank_result_t res (all_reduce( detail::command_t("CONNECT")
                                       , reduce::max_result
                                       , rank_result_t (m_rank, 0) // my result
                                       )
                            );
          if (res.value != 0)
          {
            LOG (ERROR,"connection failed: " << res.rank << " failed: " << res.value);
            throw std::runtime_error
              ( "connections could not be established to at least one node: rank "
              + boost::lexical_cast<std::string>(res.rank)
              + " says: "
              + res.message
              );
          }
          else
          {
            m_established = true;
            DMLOG(TRACE, "topology established");
          }
        }
        catch (std::exception const & ex)
        {
          throw;
        }
      }

      void topology_t::cast( const gpi::rank_t rnk
                           , const std::string & data
                           )
      {
        lock_type lock(m_mutex);
        child_map_t::const_iterator it(m_children.find(rnk));
        if (it == m_children.end())
        {
          LOG( ERROR
             , "cannot send to rank " << rnk << ":"
             << " message routing not yet implemented"
             );
          throw std::runtime_error("cannot send to this rank, not a direct child and routing is not yet implemented!");
        }
        else
        {
          cast (it->second, data);
        }
      }

      void topology_t::cast( const gpi::rank_t rnk
                           , const char * data
                           , const std::size_t len
                           )
      {
        cast(rnk, std::string(data, len));
      }

      void topology_t::message_sent ( child_t & child
                                    , std::string const & data
                                    , boost::system::error_code const & ec
                                    )
      {
        if (not m_shutting_down && ec)
        {
          if (++child.error_counter > 10)
          {
            MLOG (ERROR, "exceeded error counter for " << child.name);
            m_peer->stop ();
          }
          else
          {
            usleep (child.error_counter * 200 * 1000);
            cast (child, data);
          }
        }
        else
        {
          child.error_counter = 0;
        }
      }

      void topology_t::cast( const child_t & child
                           , const std::string & data
                           )
      {
        m_peer->async_send ( child.name
                           , data
                           , boost::bind ( &topology_t::message_sent
                                         , this
                                         , child
                                         , data
                                         , _1
                                         )
                           );
      }

      void topology_t::broadcast (const std::string &data)
      {
        BOOST_FOREACH(child_map_t::value_type const & n, m_children)
        {
          cast(n.second, data);
        }
      }

      void topology_t::broadcast ( const char *data
                                 , const std::size_t len
                                 )
      {
        return broadcast(std::string(data, len));
      }

      /*
        1. map ((id, Request)) -> [(Node, Result)]
        2. fold ([(Node, Result)], (MyRank, MyResult)) -> Result
       */

      void topology_t::message_received(boost::system::error_code const &ec)
      {
        if (! ec)
        {
          const fhg::com::p2p::address_t & addr = m_incoming_msg.header.src;
          const std::string name(m_peer->resolve(addr, "*unknown*"));

          handle_message( detail::name_to_rank(name)
                        , std::string( m_incoming_msg.buf()
                                     , m_incoming_msg.header.length
                                     )
                        );

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
                                      , std::string const &msg
                                      )
      {
        DLOG(TRACE, "got message from gpi-" << rank << ": " << msg);

        // TODO: push message to message handler

        if (rank != m_rank && msg == "CONNECT")
        {
          add_child(rank); // actually set_parent(rank)?
          cast (rank, detail::command_t("+RES") << 0);

          m_established = true;
        }
        else
        {
          // split message
          std::vector<std::string> av;
          boost::algorithm::split ( av, msg
                                  , boost::algorithm::is_space()
                                  , boost::algorithm::token_compress_on
                                  );
          if (av.empty())
          {
            LOG (ERROR, "ignoring empty command");
          }
          else if (av[0] == "ALLOC")
          {
            using namespace gpi::pc::type;
            segment_id_t seg (boost::lexical_cast<segment_id_t>(av[1]));
            handle_t hdl (boost::lexical_cast<handle_t>(av[2]));
            offset_t offset (boost::lexical_cast<offset_t>(av[3]));
            size_t   size (boost::lexical_cast<size_t>(av[4]));
            size_t local_size (boost::lexical_cast<size_t>(av[5]));
            std::string name
              (boost::algorithm::trim_copy_if( av[6]
                                             , boost::is_any_of("\"")
                                             )
              ); // TODO unquote and join (av[4]...)

            try
            {
              int res
                (global::memory_manager().remote_alloc( seg
                                                      , hdl
                                                      , offset
                                                      , size
                                                      , local_size
                                                      , name
                                                      )
                );
              cast (rank, detail::command_t("+RES") << res);
            }
            catch (std::exception const &ex)
            {
              cast (rank, detail::command_t("+RES") << 2 << ex.what ());
            }
          }
          else if (av[0] == "FREE")
          {
            using namespace gpi::pc::type;
            handle_t hdl (boost::lexical_cast<handle_t>(av[1]));
            try
            {
              global::memory_manager().remote_free(hdl);
              cast (rank, detail::command_t("+OK"));
            }
            catch (std::exception const & ex)
            {
              MLOG_IF ( WARN
                      , not m_shutting_down
                      , "could not free handle: " << ex.what()
                      );
              cast (rank, detail::command_t("+ERR") << 1 << ex.what ());
            }
          }
          else if (av [0] == "ADDMEM")
          {
            using namespace gpi::pc::type;
            segment_id_t seg_id = boost::lexical_cast<segment_id_t> (av[1]);
            std::string url_s
              (boost::algorithm::trim_copy_if( av [2]
                                             , boost::is_any_of("\"")
                                             )
              ); // TODO unquote and join (av[4]...)

            try
            {
              global::memory_manager().remote_add_memory (seg_id, url_s);
              cast (rank, detail::command_t("+RES") << 0);
            }
            catch (std::exception const & ex)
            {
              MLOG( ERROR
                  , "add_memory(" << seg_id << ", '" << url_s << "')"
                  << " failed: " << ex.what()
                  );
              cast (rank, detail::command_t("+RES") << 1 << ex.what ());
            }
          }
          else if (av [0] == "DELMEM")
          {
            using namespace gpi::pc::type;
            segment_id_t seg_id = boost::lexical_cast<segment_id_t> (av[1]);

            try
            {
              global::memory_manager().remote_del_memory (seg_id);
              cast (rank, detail::command_t("+RES") << 0);
            }
            catch (std::exception const & ex)
            {
              MLOG( ERROR
                  , "del_memory(" << seg_id <<  ")"
                  << " failed: " << ex.what()
                  );
              cast (rank, detail::command_t("+RES") << 1 << ex.what ());
            }
          }
          else if (av[0] == "+RES")
          {
            lock_type lck(m_result_mutex);
            std::vector<std::string> msg_vec ( av.begin ()+2
                                             , av.end ()
                                             );
            m_current_results.push_back
              (rank_result_t( rank
                            , boost::lexical_cast<int>(av[1])
                            , boost::algorithm::join (msg_vec, " ")
                            )
              );
            m_request_finished.notify_one();
          }
          else if (av[0] == "+OK")
          {
            DLOG(TRACE, "command succeeded");
          }
          else if (av[0] == "+ERR")
          {
            std::vector<std::string> msg_vec ( av.begin ()+2
                                             , av.end ()
                                             );
            MLOG_IF ( WARN
                    , not m_shutting_down
                    , "error on node " << rank
                    << ": " << av [1]
                    << ": " << boost::algorithm::join (msg_vec, " ")
                    );
          }
          else if (av [0] == "GO")
          {
            lock_type lck (m_go_event_mutex);
            m_go_received = true;
            m_go_received_event.notify_all ();
          }
          else if (av[0] == "SHUTDOWN" && !m_shutting_down)
          {
            LOG(INFO, "shutting down");
            m_children.clear();
            m_shutting_down = true;
            kill(getpid(), SIGTERM);
          }
          else
          {
            LOG(WARN, "invalid command: '" << av[0] <<"'");
          }
        }
      }

      void topology_t::handle_error ( const gpi::rank_t rank
                                    , boost::system::error_code const &ec
                                    )
      {
        if (m_established || m_waiting_for_go)
        {
          MLOG (DEBUG, "error on connection to " << rank << ": " << ec);
          MLOG (DEBUG, "node-failover is not available yet, I have to commit Seppuku...");
          m_shutting_down = true;
          if (m_waiting_for_go)
          {
            m_go_received_event.notify_all ();
          }

          del_child (rank);

          // delete my kvs entry and the one from the child in case it couldn't
          gpi::rank_t rnks [] = { m_rank , rank };
          for (size_t i = 0 ; i < 2 ; ++i)
          {
            std::string peer_name = fhg::com::p2p::to_string
              (fhg::com::p2p::address_t (detail::rank_to_name (rnks[i])));
            std::string kvs_key = "p2p.peer." + peer_name;
            fhg::com::kvs::del (kvs_key);
          }

          kill(getpid(), SIGTERM);
        }
      }
    }
  }
}
