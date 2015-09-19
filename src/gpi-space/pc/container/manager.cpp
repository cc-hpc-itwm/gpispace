
#include <gpi-space/pc/container/manager.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/variant/static_visitor.hpp>

#include <util-generic/syscall.hpp>
#include <util-generic/nest_exceptions.hpp>

#include <fhglog/LogMacros.hpp>

#include <gpi-space/gpi/api.hpp>
#include <gpi-space/pc/memory/shm_area.hpp>
#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/url.hpp>
#include <gpi-space/pc/url_io.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <stdexcept>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      manager_t::~manager_t ()
      {
        try
        {
          if (m_socket >= 0)
          {
            m_stopping = true;

            safe_unlink (m_path);
            close_socket (m_socket);

            _listener_thread.join();

            m_socket = -1;
          }
        }
        catch (std::exception const & ex)
        {
          LLOG(ERROR, _logger, "error within ~manager_t: " << ex.what());
        }

        std::unique_lock<std::mutex> const _ (_mutex_processes);
        for ( std::pair<gpi::pc::type::process_id_t const, std::thread>& process
            : m_processes
            )
        {
          process.second.join();

          LLOG( INFO
              , _logger
             , "process container " << process.first << " detached"
             );
        }

        _memory_manager.clear();
      }

      void manager_t::close_socket (const int fd)
      {
        fhg::util::syscall::shutdown (fd, SHUT_RDWR);
        fhg::util::syscall::close (fd);
      }

      void manager_t::safe_unlink(std::string const & path)
      {
        struct stat st;

        try
        {
          fhg::util::syscall::stat (path.c_str(), &st);
        }
        catch (boost::system::system_error const&)
        {
          return;
        }

        if (!S_ISSOCK(st.st_mode))
        {
          throw std::runtime_error ("not a socket");
        }

        fhg::util::syscall::unlink (path.c_str());
      }

      void manager_t::listener_thread_main()
      {
        int cfd;
        struct sockaddr_un peer_addr;
        socklen_t peer_addr_size;

        for (;;)
        {
          peer_addr_size = sizeof(struct sockaddr_un);
          try
          {
            cfd = fhg::util::syscall::accept ( m_socket
                                             , (struct sockaddr*)&peer_addr
                                             , &peer_addr_size
                                             );
          }
          catch (boost::system::system_error const& se)
          {
            if (m_stopping)
            {
              break;
            }
            else
            {
              continue;
            }
          }

            gpi::pc::type::process_id_t const id (++m_process_counter);

            {
              std::unique_lock<std::mutex> const _ (_mutex_processes);

              m_processes.emplace
                ( id
                , std::thread
                    (&manager_t::process_communication_thread, this, id, cfd)
                );
            }

            LLOG( INFO
                , _logger
               , "process container " << id << " attached"
               );
        }
      }

      namespace
      {
        struct handle_message_t : public boost::static_visitor<gpi::pc::proto::message_t>
        {
          handle_message_t ( fhg::log::Logger& logger
                           , gpi::pc::type::process_id_t const& proc_id
                           , memory::manager_t& memory_manager
                           , global::topology_t& topology
                           )
            : _logger (logger)
            , m_proc_id (proc_id)
            , _memory_manager (memory_manager)
            , _topology (topology)
          {}

          /**********************************************/
          /***     M E M O R Y   R E L A T E D        ***/
          /**********************************************/

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::alloc_t & alloc) const
          {
            gpi::pc::proto::memory::alloc_reply_t rpl;
            rpl.handle = _memory_manager.alloc
              ( m_proc_id
              , alloc.segment, alloc.size, alloc.name, alloc.flags
              );
            return gpi::pc::proto::memory::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::free_t & free) const
          {
            _memory_manager.free (free.handle);
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::success, "success");
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::memcpy_t & cpy) const
          {
            gpi::pc::type::validate (cpy.dst.handle);
            gpi::pc::type::validate (cpy.src.handle);
            gpi::pc::proto::memory::memcpy_reply_t rpl;
            rpl.queue = _memory_manager.memcpy (cpy.dst, cpy.src, cpy.size);
            return gpi::pc::proto::memory::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::wait_t & w) const
          {
            gpi::pc::proto::memory::wait_reply_t rpl;
            // this is not that easy to implement
            //    do we want to put the process container to sleep? - no
            //    we basically want to delay the answer
            //    the client should also be able to release the lock so that other threads can still interact with the pc
            //
            // TODO:
            //   client:
            //     attach a unique sequence number to the wait message
            //     enqueue the request
            //     send the message
            //     unlock communication lock
            //     wait for the request to "return"
            //
            //   server:
            //     enqueue the "wait" request into some queue that is handled by a seperate thread (each queue one thread)
            //     "reply" to the enqueued wait request via this process using the same sequence number waking up the client thread
            //
            //   implementation:
            //     processes' message visitor has to be rewritten to be "asynchronous" in some way
            //        -> idea: visitor's return value is a boost::optional
            //                 if set: reply immediately
            //                   else: somebody else will reply later
            //           messages need unique sequence numbers or message-ids
            rpl.count = _memory_manager.wait_on_queue
              (m_proc_id, w.queue);
            return gpi::pc::proto::memory::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::info_t & info) const
          {
            gpi::pc::proto::memory::info_reply_t rpl;
            rpl.descriptor = _memory_manager.info (info.handle);
            return gpi::pc::proto::memory::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator() (const gpi::pc::proto::memory::get_transfer_costs_t& request) const
          {
            return gpi::pc::proto::memory::message_t
              (gpi::pc::proto::memory::transfer_costs_t (_memory_manager.get_transfer_costs (request.transfers)));
          }

          /**********************************************/
          /***     S E G M E N T   R E L A T E D      ***/
          /**********************************************/

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::register_t & register_segment) const
          {
            url_t url ("shm", register_segment.name);
            url.set ("size", boost::lexical_cast<std::string>(register_segment.size));
            if (register_segment.flags & F_PERSISTENT)
              url.set ("persistent", "true");
            if (register_segment.flags & F_EXCLUSIVE)
              url.set ("exclusive", "true");

            memory::area_ptr_t area (memory::shm_area_t::create (_logger, boost::lexical_cast<std::string>(url), _memory_manager.handle_generator()));
            area->set_owner (m_proc_id);

            gpi::pc::proto::segment::register_reply_t rpl;
            rpl.id =
              _memory_manager.register_memory (m_proc_id, area);

            return gpi::pc::proto::segment::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::unregister_t & unregister_segment) const
          {
            _memory_manager.unregister_memory
              (m_proc_id, unregister_segment.id);
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::success, "success");
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::attach_t & attach_segment) const
          {
            _memory_manager.attach_process
              (m_proc_id, attach_segment.id);
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::success, "success");
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::detach_t & detach_segment) const
          {
            _memory_manager.detach_process
              (m_proc_id, detach_segment.id);
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::success, "success");
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::add_memory_t & add_mem) const
          {
            gpi::pc::type::segment_id_t id =
              _memory_manager.add_memory (m_proc_id, add_mem.url, 0, _topology);
            gpi::pc::proto::segment::register_reply_t rpl;
            rpl.id = id;
            return gpi::pc::proto::segment::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::del_memory_t & del_mem) const
          {
            _memory_manager.del_memory (m_proc_id, del_mem.id, _topology);
            return
              gpi::pc::proto::error::error_t (gpi::pc::proto::error::success);
          }

          /**********************************************/
          /***     C O N T R O L   R E L A T E D      ***/
          /**********************************************/

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::message_t & m) const
          {
            return boost::apply_visitor (*this, m);
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::message_t & m) const
          {
            return boost::apply_visitor (*this, m);
          }

          /*** Catch all other messages ***/

          template <typename T>
            gpi::pc::proto::message_t
            operator () (T const &) const
          {
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::bad_request,  "invalid input message");
          }

        private:
          fhg::log::Logger& _logger;
          gpi::pc::type::process_id_t const& m_proc_id;
          memory::manager_t& _memory_manager;
          global::topology_t& _topology;
        };

        gpi::pc::proto::message_t handle_message
          ( fhg::log::Logger& logger
          , gpi::pc::type::process_id_t const& id
          , gpi::pc::proto::message_t const& request
          , gpi::pc::memory::manager_t& memory_manager
          , global::topology_t& topology
          )
        {
          try
          {
            return boost::apply_visitor
              (handle_message_t (logger, id, memory_manager, topology), request);
          }
          catch (std::exception const& ex)
          {
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::bad_request, ex.what());
          }
        }

        void read_exact (int fd, void* buffer, size_t size)
        {
          if (fhg::util::syscall::read (fd, buffer, size) != size)
          {
            throw std::runtime_error
              ("unable to read " + std::to_string (size) + " bytes");
          }
        }
        void write_exact (int fd, void const* buffer, size_t size)
        {
          if (fhg::util::syscall::write (fd, buffer, size) != size)
          {
            throw std::runtime_error
              ("unable to write " + std::to_string (size) + " bytes");
          }
        }
      }

      void manager_t::process_communication_thread
        (gpi::pc::type::process_id_t process_id, int socket)
      {
        LLOG(TRACE, _logger, "process container (" << process_id << ") started on socket " << socket);

        for (;;)
        {
          try
          {
            gpi::pc::proto::message_t request;

            fhg::util::nest_exceptions<std::runtime_error>
              ( [&]
                {
                  gpi::pc::proto::header_t header;
                  read_exact (socket, &header, sizeof (header));

                  std::vector<char> buffer (header.length);
                  read_exact (socket, buffer.data(), buffer.size());

                  fhg::util::nest_exceptions<std::runtime_error>
                    ( [&]
                      {
                        std::stringstream sstr
                          (std::string (buffer.data(), buffer.size()));
                        boost::archive::binary_iarchive ia (sstr);
                        ia & request;
                      }
                    , "could not decode message"
                    );
                }
              , "could not receive message"
              );

            gpi::pc::proto::message_t const reply
              (handle_message (_logger, process_id, request, _memory_manager, _topology));

            fhg::util::nest_exceptions<std::runtime_error>
              ( [&]
                {
                  std::string data;
                  gpi::pc::proto::header_t header;

                  fhg::util::nest_exceptions<std::runtime_error>
                    ( [&]
                      {
                        std::stringstream sstr;
                        boost::archive::text_oarchive oa (sstr);
                        oa & reply;
                        data = sstr.str();
                      }
                    , "could not encode message"
                    );

                  header.length = data.size();

                  write_exact (socket, &header, sizeof (header));
                  write_exact (socket, data.data(), data.size());
                }
              , "could not send message"
              );
          }
          catch (std::exception const & ex)
          {
            LLOG(ERROR, _logger, "process container " << process_id << " crashed: " << ex.what());
            break;
          }
        }

        try
        {
          fhg::util::syscall::shutdown (socket, SHUT_RDWR);
        }
        catch (boost::system::system_error const& se)
        {
          if (se.code() != boost::system::errc::not_connected)
          {
            //! \note ignored: already disconnected = fine, we terminate
            throw;
          }
        }

        fhg::util::syscall::close (socket);

        _memory_manager.garbage_collect (process_id);

        LLOG(TRACE, _logger, "process container (" << process_id << ") terminated");

        //! \note this detaches _this_ thread from everything
        //! left. Nothing shall be done in here that accesses `this`
        //! after the erase, or does anything! Let this thing die!

        if (!m_stopping)
        {
          std::unique_lock<std::mutex> const _ (_mutex_processes);
          m_processes.at (process_id).detach();
          m_processes.erase (process_id);
        }
      }


      manager_t::manager_t ( fhg::log::Logger& logger
                           , std::string const & p
                           , std::vector<std::string> const& default_memory_urls
                           , api::gpi_api_t& gpi_api
                           , std::unique_ptr<fhg::com::peer_t> topology_peer
                           )
        : _logger (logger)
        , m_path (p)
        , m_socket (-1)
        , m_stopping (false)
        , m_process_counter (0)
        , _memory_manager (_logger, gpi_api)
        , _topology (_logger, _memory_manager, gpi_api, std::move (topology_peer))
      {
        if ( default_memory_urls.size ()
           >= gpi::pc::memory::manager_t::MAX_PREALLOCATED_SEGMENT_ID
           )
        {
          throw std::runtime_error ("too many predefined memory urls!");
        }

        if (_topology.is_master ())
        {
          gpi::pc::type::id_t id = 1;
          for (std::string const& url : default_memory_urls)
          {
            _memory_manager.add_memory
              ( 0 // owner
              , url
              , id
              , _topology
              );
            ++id;
          }
        }

        fhg::util::nest_exceptions<std::runtime_error>
          ( [&]
            {
              safe_unlink (m_path);
            }
          , "could not unlink path '" + m_path + "'"
          );

        fhg::util::nest_exceptions<std::runtime_error>
          ( [&]
            {
              m_socket = fhg::util::syscall::socket (AF_UNIX, SOCK_STREAM, 0);
            }
          , "could not create process-container communication socket"
          );

        struct close_socket_on_error
        {
          ~close_socket_on_error()
          {
            if (!_committed)
            {
              fhg::util::syscall::close (_socket);
            }
          }
          bool _committed;
          int& _socket;
        } close_socket_on_error = {false, m_socket};

        {
          const int on (1);
          fhg::util::syscall::setsockopt (m_socket, SOL_SOCKET, SO_PASSCRED, &on, sizeof (on));
        }

        struct sockaddr_un my_addr;
        memset (&my_addr, 0, sizeof(my_addr));
        my_addr.sun_family = AF_UNIX;
        strncpy ( my_addr.sun_path
                , m_path.c_str()
                , sizeof(my_addr.sun_path) - 1
                );

        fhg::util::nest_exceptions<std::runtime_error>
          ( [&]
            {
              fhg::util::syscall::bind
                (m_socket, (struct sockaddr *)&my_addr, sizeof (struct sockaddr_un));
            }
          , "could not bind process-container communication socket to path " + m_path
          );

        struct delete_socket_file_on_error
        {
          ~delete_socket_file_on_error()
          {
            if (!_committed)
            {
              fhg::util::syscall::unlink (_path.string().c_str());
            }
          }
          bool _committed;
          boost::filesystem::path _path;
        } delete_socket_file_on_error = {false, m_path};

        fhg::util::syscall::chmod (m_path.c_str(), 0700);

        const std::size_t backlog_size (16);
        fhg::util::syscall::listen (m_socket, backlog_size);

        _listener_thread = std::thread (&manager_t::listener_thread_main, this);

        delete_socket_file_on_error._committed = true;
        close_socket_on_error._committed = true;
      }
    }
  }
}
