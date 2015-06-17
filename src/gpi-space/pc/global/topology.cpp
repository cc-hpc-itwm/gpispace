#include <gpi-space/pc/global/topology.hpp>

#include <unistd.h>
#include <stdio.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

#include <gpi-space/log_to_GLOBAL_logger.hpp>
#include <fhg/assert.hpp>
#include <fhg/util/join.hpp>
#include <util-generic/print_exception.hpp>

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
                             , std::unique_ptr<fhg::com::peer_t> peer
                             )
        : m_shutting_down (false)
        , _gpi_api (gpi_api)
        , m_peer (std::move (peer))
      {
        std::unique_lock<std::mutex> const _ (m_mutex);

        // start the message handler
        m_peer->async_recv ( &m_incoming_msg
                           , std::bind( &topology_t::message_received
                                      , this
                                      , std::placeholders::_1
                                      , std::placeholders::_2
                                      , std::ref (memory_manager)
                                      )
                           );

        for (std::size_t rank (0); rank < _gpi_api.number_of_nodes(); ++rank)
        {
          if (_gpi_api.rank() == rank)
          {
            continue;
          }

          m_children.emplace
            ( m_peer->connect_to_or_use_existing_connection
                ( fhg::com::host_t (_gpi_api.hostname_of_rank (rank))
                , fhg::com::port_t
                    (std::to_string (_gpi_api.communication_port_of_rank (rank)))
                )
            );
        }
      }

      topology_t::~topology_t()
      {
        {
          std::unique_lock<std::mutex> const _ (m_mutex);
          m_shutting_down = true;
        }
      }

      bool topology_t::is_master () const
      {
        return 0 == _gpi_api.rank();
      }

      void topology_t::request (std::string const& name, std::string const& req)
      {
        //! \note one request at a time due to state in
        //! m_current_results and no message sequencing
        std::unique_lock<std::mutex> const _ (m_request_mutex);

        std::unique_lock<std::mutex> result_list_lock (m_result_mutex);
        m_current_results.clear ();

        for (fhg::com::p2p::address_t const& child : m_children)
        {
          cast (child, req);
        }

        if ( !m_request_finished.wait_for
               ( result_list_lock
               , std::chrono::seconds (30)
               , [&]
                 {
                   return m_current_results.size() == m_children.size();
                 }
               )
           )
        {
          throw std::runtime_error
            ( name + " failed: timed out after 30 seconds: "
            + std::to_string (m_current_results.size()) + " of "
            + std::to_string (m_children.size()) + " results received"
            );
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
        std::unique_lock<std::mutex> const _ (m_global_alloc_mutex);

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

      void topology_t::message_sent (boost::system::error_code const& ec)
      {
        if (not m_shutting_down && ec)
        {
          LOG (ERROR, "failed sending a message: " << ec);
        }
      }

      void topology_t::cast
        (fhg::com::p2p::address_t const& address, std::string const& data)
      {
        m_peer->async_send ( address
                           , data
                           , std::bind ( &topology_t::message_sent
                                       , this
                                       , std::placeholders::_1
                                       )
                           );
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
            m_shutting_down = true;
          }
        }
      }

      void topology_t::handle_message ( fhg::com::p2p::address_t const& source
                                      , std::string const &msg
                                      , memory::manager_t& memory_manager
                                      )
      {
        // TODO: push message to message handler

        try
        {
          // split message
          std::vector<std::string> av;
          boost::algorithm::split ( av, msg
                                  , boost::algorithm::is_space()
                                  , boost::algorithm::token_compress_on
                                  );
          if (av.empty())
          {
            throw std::logic_error ("empty command");
          }

          if (av[0] == "ALLOC")
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

            memory_manager.remote_alloc( seg
                                       , hdl
                                       , offset
                                       , size
                                       , local_size
                                       , name
                                       );
          }
          else if (av[0] == "FREE")
          {
            type::handle_t hdl (boost::lexical_cast<type::handle_t>(av[1]));

            memory_manager.remote_free(hdl);
          }
          else if (av [0] == "ADDMEM")
          {
            type::segment_id_t seg_id = boost::lexical_cast<type::segment_id_t> (av[1]);
            std::string url_s
              (boost::algorithm::trim_copy_if( av [2]
                                             , boost::is_any_of("\"")
                                             )
              ); // TODO unquote and join (av[4]...)

            memory_manager.remote_add_memory (seg_id, url_s, *this);
          }
          else if (av [0] == "DELMEM")
          {
            type::segment_id_t seg_id = boost::lexical_cast<type::segment_id_t> (av[1]);

            memory_manager.remote_del_memory (seg_id, *this);
          }
          else if (av[0] == "+RES")
          {
            std::unique_lock<std::mutex> const _ (m_result_mutex);

            m_current_results.emplace_back
              ( boost::make_optional
                  ( boost::lexical_cast<int> (av[1])
                  , fhg::util::join (av.begin() + 2, av.end(), " ")
                  )
              );
            m_request_finished.notify_one();

            return;
          }
          else
          {
            throw std::logic_error ("invalid command: " + av[0]);
          }

          cast (source, detail::command_t ("+RES") << 0);
        }
        catch (...)
        {
          LOG ( ERROR
              , "handling command '" + msg + "' failed: "
              << fhg::util::current_exception_printer (": ")
              );
          cast (source, detail::command_t ("+RES") << 1 << fhg::util::current_exception_as_string (": "));
        }
      }
    }
  }
}
