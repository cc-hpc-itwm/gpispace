#include <gpi-space/gpi/api.hpp>
#include <gpi-space/pc/container/manager.hpp>
#include <gpi-space/pc/container/process.hpp>
#include <gpi-space/pc/global/topology.hpp>
#include <gpi-space/pc/memory/manager.hpp>
#include <gpi-space/pc/segment/segment.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/lift_error_code_to_exception.hpp>
#include <fhg/syscall.hpp>
#include <fhglog/LogMacros.hpp>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
          stop ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "error within ~manager_t: " << ex.what());
        }

          while (! m_processes.empty())
          {
            detach_process (m_processes.begin()->first);
          }

        _memory_manager.clear();
        _topology.stop();
      }

      void manager_t::start ()
      {
        lock_type lock (m_mutex);

        if (m_socket >= 0)
          return;

        m_stopping = false;
        int err = safe_unlink (m_path);
        if (err < 0)
        {
          LOG(ERROR, "could not unlink path " << m_path << ": " <<  strerror(-err));
          throw std::runtime_error ("could not unlink socket path");
        }

        int fd (open_socket (m_path));
        if (fd < 0)
        {
          LOG(ERROR, "could not open socket: " << strerror(-fd));
          throw std::runtime_error ("could not open socket");
        }
        else
        {
          m_socket = fd;

          m_listener = thread_t
            (new boost::thread(boost::bind( &manager_t::listener_thread_main
                                          , this
                                          , m_socket
                                          )
                              )
            );
        }
      }

      void manager_t::stop ()
      {
        lock_type lock (m_mutex);
        if (m_socket >= 0)
        {
          m_stopping = true;

          safe_unlink (m_path);
          close_socket (m_socket);
          assert (m_listener);
          if (boost::this_thread::get_id() != m_listener->get_id())
          {
            m_listener->join ();
            m_listener.reset ();
          }
          m_socket = -1;
        }
      }

      void manager_t::close_socket (const int fd)
      {
        fhg::syscall::shutdown (fd, SHUT_RDWR);
        fhg::syscall::close (fd);
      }

      int manager_t::open_socket (std::string const & path)
      {
        int sfd;
        struct sockaddr_un my_addr;
        const std::size_t backlog_size (16);

        try
        {
          sfd = fhg::syscall::socket (AF_UNIX, SOCK_STREAM, 0);
        }
        catch (boost::system::system_error const& se)
        {
          LOG(ERROR, "could not create unix socket: " << se.what());
          return -se.code().value();
        }
        fhg::syscall::setsockopt (sfd, SOL_SOCKET, SO_PASSCRED, (void*)1, 1);

        memset (&my_addr, 0, sizeof(my_addr));
        my_addr.sun_family = AF_UNIX;
        strncpy ( my_addr.sun_path
                , path.c_str()
                , sizeof(my_addr.sun_path) - 1
                );

        try
        {
          fhg::syscall::bind
            (sfd, (struct sockaddr *)&my_addr, sizeof (struct sockaddr_un));
        }
        catch (boost::system::system_error const& se)
        {
          LOG(ERROR, "could not bind to socket at path " << path << ": " << se.what());
          fhg::syscall::close (sfd);
          return -se.code().value();
        }
        fhg::syscall::chmod (path.c_str(), 0700);

        try
        {
          fhg::syscall::listen (sfd, backlog_size);
        }
        catch (boost::system::system_error const& se)
        {
          LOG(ERROR, "could not listen on socket: " << se.what());
          fhg::syscall::close (sfd);
          fhg::syscall::unlink (path.c_str());
          return -se.code().value();
        }

        return sfd;
      }

      int manager_t::safe_unlink(std::string const & path)
      {
        struct stat st;

        try
        {
          fhg::syscall::stat (path.c_str(), &st);
        }
        catch (boost::system::system_error const&)
        {
          return 0;
        }

        if (S_ISSOCK(st.st_mode))
        {
          fhg::syscall::unlink (path.c_str());
          return 0;
        }
        else
        {
          return -EINVAL;
        }
      }

      void manager_t::listener_thread_main(const int fd)
      {
        int cfd;
        struct sockaddr_un peer_addr;
        socklen_t peer_addr_size;

        for (;;)
        {
          peer_addr_size = sizeof(struct sockaddr_un);
          try
          {
            cfd = fhg::syscall::accept ( fd
                                       , (struct sockaddr*)&peer_addr
                                       , &peer_addr_size
                                       );
          }
          catch (boost::system::system_error const& se)
          {
            if (! m_stopping)
            {
              LOG(ERROR, "could not accept: " << se.what());

              if (se.code().value())
              {
                LOG( ERROR
                   , "connector had an error: " << se.what() << ", restarting it"
                   );

                stop ();
                start ();
              }
            }
            break;
          }

          try
          {
            gpi::pc::type::process_id_t const id (m_process_counter.inc());

            {
              boost::mutex::scoped_lock const _ (_mutex_processes);

              m_processes.insert
                ( id
                , std::auto_ptr<process_t>
                  ( new process_t
                    ( boost::bind (&manager_t::handle_process_error, this, _1, _2)
                    , id
                    , fd
                    , _memory_manager
                    , _topology
                    )
                  )
                );
            }

            CLOG( INFO
                , "gpi.container"
                , "process container " << id << " attached"
                );
          }
          catch (std::exception const & ex)
          {
            LOG(ERROR, "could not handle new connection: " << ex.what());
            close_socket (cfd);
          }
        }
      }

      manager_t::manager_t
        ( std::string const & p
        , std::vector<std::string> const& default_memory_urls
        )
          : m_path (p)
          , m_socket (-1)
          , m_stopping (false)
          , m_process_counter (0)
          , _memory_manager()
          , _topology()
      {
        if ( default_memory_urls.size ()
           >= gpi::pc::memory::manager_t::MAX_PREALLOCATED_SEGMENT_ID
           )
        {
          throw std::runtime_error ("too many predefined memory urls!");
        }

        gpi::api::gpi_api_t & gpi_api (gpi::api::gpi_api_t::get());
        _topology.start( gpi_api.rank()
                                , global::topology_t::any_addr()
                                , global::topology_t::any_port() // topology_t::port_t("10821")
                                , "dummy-cookie"
                                , _memory_manager
                                );

        for (std::size_t n(0); n < gpi_api.number_of_nodes(); ++n)
        {
          if (gpi_api.rank() != n)
            _topology.add_child(n);
        }

        if (gpi_api.is_master ())
        {
          _topology.establish();
        }
        _memory_manager.start ( gpi_api.rank ()
                                        , gpi_api.number_of_queues ()
                                        );

        if (_topology.is_master ())
        {
          gpi::pc::type::id_t id = 1;
          BOOST_FOREACH (std::string const& url, default_memory_urls)
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

          if (_topology.is_master ())
          {
            _topology.go ();
          }
          else
          {
            _topology.wait_for_go ();
          }

          start ();
      }

      void manager_t::detach_process (const gpi::pc::type::process_id_t id)
      {
        boost::mutex::scoped_lock const _ (_mutex_processes);

        if (m_processes.find (id) == m_processes.end())
        {
          CLOG( ERROR
              , "gpi.container"
              , "process id already detached!"
              );
          throw std::runtime_error ("no such process");
        }

        m_processes.erase (id);

        _memory_manager.garbage_collect (id);

        CLOG( INFO
            , "gpi.container"
            , "process container " << id << " detached"
            );
      }

      void manager_t::handle_process_error( const gpi::pc::type::process_id_t proc_id
                                          , int error
                                          )
      {
          LOG_IF ( ERROR
                 , error != 0
                 , "process container " << proc_id << " died: " << strerror (error)
                 );
          detach_process (proc_id);
      }
    }
  }
}
