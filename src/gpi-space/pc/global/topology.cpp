#include <gpi-space/pc/global/topology.hpp>

#include <unistd.h>
#include <stdio.h>
#include <csignal> // kill

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>
#include <fhg/util/join.hpp>

#include <gpi-space/gpi/api.hpp>
#include <gpi-space/pc/memory/manager.hpp>

#include <functional>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      namespace detail
      {
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

      topology_t::topology_t ( memory::manager_t& memory_manager
                             , api::gpi_api_t& gpi_api
                             , boost::shared_ptr<fhg::com::peer_t> const& peer
                             )
        : m_shutting_down (false)
        , m_rank (gpi_api.rank())
        , m_peer (peer)
        , _gpi_api (gpi_api)
      {
        lock_type lock(m_mutex);

        // start the message handler
        m_peer->async_recv ( &m_incoming_msg
                           , std::bind( &topology_t::message_received
                                      , this
                                      , std::placeholders::_1
                                      , std::placeholders::_2
                                      , std::ref (memory_manager)
                                      )
                           );

        for (std::size_t n(0); n < _gpi_api.number_of_nodes(); ++n)
        {
          if (_gpi_api.rank() != n)
            add_child(n);
        }
      }

      topology_t::~topology_t()
      {
        {
          lock_type lock(m_mutex);
          m_shutting_down = true;
        }
      }

      bool topology_t::is_master () const
      {
        return 0 == m_rank;
      }

      void topology_t::add_child(const gpi::rank_t rank)
      {
        child_t new_child(rank);

        new_child.address =
          m_peer->connect_to_or_use_existing_connection
            ( fhg::com::host_t (_gpi_api.hostname_of_rank (rank))
            , fhg::com::port_t
                (std::to_string (_gpi_api.communication_port_of_rank (rank)))
            );

        lock_type lock(m_mutex);
        fhg_assert (m_peer);
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

      void topology_t::request (std::string const& name, std::string const& req)
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
              throw std::runtime_error
                (name + " failed: timed out after 30 seconds!");
            }
          }
        }

        std::vector<std::string> errors;
        for (boost::optional<std::string>& partial_result : m_current_results)
        {
          if (partial_result)
          {
            errors.emplace_back (std::move (*partial_result));
          }
        }

        if (!errors.empty())
        {
          LOG (ERROR, name << " failed: " << fhg::util::join (errors, "; "));
          throw std::runtime_error
            (name + " failed: " + fhg::util::join (errors, "; "));
        }
      }

      void topology_t::free (const gpi::pc::type::handle_t hdl)
      {
        request ("free", detail::command_t ("FREE") << hdl);
      }

      void topology_t::alloc ( const gpi::pc::type::segment_id_t seg
                             , const gpi::pc::type::handle_t hdl
                             , const gpi::pc::type::offset_t offset
                             , const gpi::pc::type::size_t size
                             , const gpi::pc::type::size_t local_size
                             , const std::string & name
                             )
      {
        // lock, so that no other process can make a global alloc
        lock_type alloc_lock(m_global_alloc_mutex);

        try
        {
          request ( "alloc"
                  , detail::command_t ("ALLOC")
                  << seg << hdl << offset << size << local_size << name
                  );
        }
        catch (...)
        {
          free (hdl);
          throw;
        }
      }

      void topology_t::add_memory ( const gpi::pc::type::segment_id_t seg_id
                                  , const std::string & url
                                  )
      {
        request ("add_memory", detail::command_t ("ADDMEM") << seg_id << url);
      }

      void topology_t::del_memory (const gpi::pc::type::segment_id_t seg_id)
      {
        request ("del_memory", detail::command_t ("DELMEM") << seg_id);
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

      void topology_t::message_sent ( child_t & child
                                    , std::string const & data
                                    , boost::system::error_code const & ec
                                    )
      {
        if (not m_shutting_down && ec)
        {
          if (++child.error_counter > 10)
          {
            MLOG (ERROR, "exceeded error counter for rank " << child.rank);
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
        m_peer->async_send ( child.address
                           , data
                           , std::bind ( &topology_t::message_sent
                                       , this
                                       , child
                                       , data
                                       , std::placeholders::_1
                                       )
                           );
      }

      void topology_t::cast
        (fhg::com::p2p::address_t const& address, std::string const& data)
      {
        m_peer->async_send ( address
                           , data
                           , std::bind ( &topology_t::message_sent
                                       , this
                                       , m_children.at (find_rank (address))
                                       , data
                                       , std::placeholders::_1
                                       )
                           );
      }

      void topology_t::broadcast (const std::string &data)
      {
        for (child_map_t::value_type const & n : m_children)
        {
          cast(n.second, data);
        }
      }

      /*
        1. map ((id, Request)) -> [(Node, Result)]
        2. fold ([(Node, Result)], (MyRank, MyResult)) -> Result
       */

      void topology_t::message_received
        ( boost::system::error_code const &ec
        , boost::optional<fhg::com::p2p::address_t>
        , memory::manager_t& memory_manager
        )
      {
        if (! ec)
        {
          handle_message( m_incoming_msg.header.src
                        , std::string( m_incoming_msg.buf()
                                     , m_incoming_msg.header.length
                                     )
                        , memory_manager
                        );

          m_peer->async_recv ( &m_incoming_msg
                             , std::bind( &topology_t::message_received
                                        , this
                                        , std::placeholders::_1
                                        , std::placeholders::_2
                                        , std::ref (memory_manager)
                                        )
                             );
        }
        else if (! m_shutting_down)
        {
          if (m_incoming_msg.header.src != m_peer->address())
          {
            handle_error (m_incoming_msg.header.src);

            m_peer->async_recv ( &m_incoming_msg
                               , std::bind( &topology_t::message_received
                                          , this
                                          , std::placeholders::_1
                                          , std::placeholders::_2
                                          , std::ref (memory_manager)
                                          )
                               );
          }
        }
        else
        {
          LOG(TRACE, "topology on " << m_rank << " is shutting down");
        }
      }

      void topology_t::handle_message ( fhg::com::p2p::address_t const& source
                                      , std::string const &msg
                                      , memory::manager_t& memory_manager
                                      )
      {
        // TODO: push message to message handler

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
            type::segment_id_t seg (boost::lexical_cast<type::segment_id_t>(av[1]));
            type::handle_t hdl (boost::lexical_cast<type::handle_t>(av[2]));
            type::offset_t offset (boost::lexical_cast<type::offset_t>(av[3]));
            type::size_t   size (boost::lexical_cast<type::size_t>(av[4]));
            type::size_t local_size (boost::lexical_cast<type::size_t>(av[5]));
            std::string name
              (boost::algorithm::trim_copy_if( av[6]
                                             , boost::is_any_of("\"")
                                             )
              ); // TODO unquote and join (av[4]...)

            try
            {
              int res
                (memory_manager.remote_alloc( seg
                                                      , hdl
                                                      , offset
                                                      , size
                                                      , local_size
                                                      , name
                                                      )
                );
              cast (source, detail::command_t("+RES") << res);
            }
            catch (std::exception const &ex)
            {
              cast (source, detail::command_t("+RES") << 2 << ex.what ());
            }
          }
          else if (av[0] == "FREE")
          {
            type::handle_t hdl (boost::lexical_cast<type::handle_t>(av[1]));
            try
            {
              memory_manager.remote_free(hdl);
              cast (source, detail::command_t("+RES") << 0);
            }
            catch (std::exception const & ex)
            {
              MLOG_IF ( WARN
                      , not m_shutting_down
                      , "could not free handle: " << ex.what()
                      );
              cast (source, detail::command_t("+RES") << 1 << ex.what ());
            }
          }
          else if (av [0] == "ADDMEM")
          {
            type::segment_id_t seg_id = boost::lexical_cast<type::segment_id_t> (av[1]);
            std::string url_s
              (boost::algorithm::trim_copy_if( av [2]
                                             , boost::is_any_of("\"")
                                             )
              ); // TODO unquote and join (av[4]...)

            try
            {
              memory_manager.remote_add_memory (seg_id, url_s, *this);
              cast (source, detail::command_t("+RES") << 0);
            }
            catch (std::exception const & ex)
            {
              MLOG( ERROR
                  , "add_memory(" << seg_id << ", '" << url_s << "')"
                  << " failed: " << ex.what()
                  );
              cast (source, detail::command_t("+RES") << 1 << ex.what ());
            }
          }
          else if (av [0] == "DELMEM")
          {
            type::segment_id_t seg_id = boost::lexical_cast<type::segment_id_t> (av[1]);

            try
            {
              memory_manager.remote_del_memory (seg_id, *this);
              cast (source, detail::command_t("+RES") << 0);
            }
            catch (std::exception const & ex)
            {
              MLOG( ERROR
                  , "del_memory(" << seg_id <<  ")"
                  << " failed: " << ex.what()
                  );
              cast (source, detail::command_t("+RES") << 1 << ex.what ());
            }
          }
          else if (av[0] == "+RES")
          {
            lock_type const lck (m_result_mutex);

            std::vector<std::string> const msg_vec (av.begin() + 2, av.end());
            m_current_results.emplace_back
              ( boost::make_optional
                  ( boost::lexical_cast<int> (av[1])
                  , fhg::util::join (av.begin() + 2, av.end(), " ")
                  )
              );
            m_request_finished.notify_one();
          }
          else
          {
            LOG(WARN, "invalid command: '" << av[0] <<"'");
          }
      }

      void topology_t::handle_error (fhg::com::p2p::address_t const& source)
      {
        gpi::rank_t rank (find_rank (source));
          m_shutting_down = true;

          del_child (rank);

          kill(getpid(), SIGTERM);
        }

      gpi::rank_t topology_t::find_rank (fhg::com::p2p::address_t address) const
      {
        decltype (m_children)::const_iterator child
          ( std::find_if
              ( m_children.cbegin(), m_children.cend()
              , [&address] (decltype (m_children)::value_type const& elem)
                {
                  return elem.second.address == address;
                }
              )
          );
        return child != m_children.end()
          ? child->first
          : throw std::runtime_error ("find_rank (unknown address)");
      }
    }
  }
}
