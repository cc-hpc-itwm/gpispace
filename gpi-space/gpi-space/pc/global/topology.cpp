#include "topology.hpp"

#include <unistd.h>
#include <stdio.h>
#include <csignal> // kill

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/assert.hpp>

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
        static fhg::com::host_t h(boost::asio::ip::host_name());
        return h;
      }

      topology_t::topology_t()
        : m_shutting_down (false)
        , m_rank ((gpi::rank_t)-1)
        , m_established (false)
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
                                       , rank_result_t(m_rank, 0) // my result
                                       )
                            );
          if (res.value != 0)
          {
            LOG(ERROR,"allocation on node " << res.rank << " failed: " << res.value);
            throw std::runtime_error("global allocation failed on at least one node");
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

        // this barrier is nice, but barriers are broken: see ticket #251
        //        gpi::api::gpi_api_t::get().barrier();

        BOOST_FOREACH(child_map_t::value_type const & n, m_children)
        {
          useconds_t snooze(500 * 1000);
          int i = 30;
          while (i --> 0)
          {
            try
            {
              LOG(TRACE, "trying to connect to " << n.second.name);
              m_peer->send (n.second.name, detail::command_t("CONNECT"));
              break;
            }
            catch (std::exception const & ex)
            {
              if (i > 0)
              {
                usleep (snooze);
                snooze = std::min(10 * 1000 * 1000u, snooze*2);
              }
              else
              {
                LOG( WARN
                   , "could not establish connection to rank " << n.first
                   << ": " << ex.what()
                   );
                throw;
              }
            }
          }
        }

        m_established = true;

        LOG(INFO, "topology established");
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

      static void message_sent (boost::system::error_code const &ec)
      {
        DLOG_IF(WARN, ec, "message could not be sent: " << ec);
      }

      void topology_t::cast( const child_t & child
                           , const std::string & data
                           )
      {
        m_peer->async_send(child.name, data, &message_sent);
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
          cast (rank, detail::command_t("+OK"));
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
              LOG(WARN, "could not free handle: " << ex.what());
              cast (rank, detail::command_t("+ERR") << 1);
            }
          }
          else if (av[0] == "+RES")
          {
            lock_type lck(m_result_mutex);
            m_current_results.push_back
              (rank_result_t( rank
                            , boost::lexical_cast<int>(av[1])
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
            LOG(WARN, "error on node " << rank << ": " << av[1]);
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
            LOG(WARN, "result collection not implemented");
          }
        }
      }

      void topology_t::handle_error ( const gpi::rank_t rank
                                    , boost::system::error_code const &ec
                                    )
      {
        if (m_established)
        {
          LOG(WARN, "error on connection to child node " << rank);
          LOG(ERROR, "node-failover is not available yet, I have to commit Seppuku...");
          del_child (rank);
          kill(getpid(), SIGTERM);
          //_exit(15);
        }
      }
    }
  }
}
