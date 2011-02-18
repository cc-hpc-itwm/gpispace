#include "signal_handler.hpp"

#include <signal.h> // sigwait...
#include <string.h> // strerror_r
#include <errno.h>  // errno
#include <pthread.h> // pthread_sigmask

#include <fhglog/minimal.hpp>

namespace gpi
{
  namespace signal
  {
    namespace {
      std::string show (const siginfo_t & info)
      {
        std::ostringstream sstr;
        sstr << "information about signal " << info.si_signo << std::endl;
        sstr << "   signal = " << info.si_signo << std::endl;
        sstr << "   errno  = " << info.si_errno << std::endl;
        sstr << "   code   = " << info.si_code  << std::endl;
        sstr << "   pid    = " << info.si_pid   << std::endl;
        sstr << "   uid    = " << info.si_uid   << std::endl;
        return sstr.str();
      }
    }

    handler_t::handler_t ()
      : m_next_connection_id (0)
      , m_signals_in_progress (0)
      , m_stopping (false)
      , m_started (false)
    {}

    // API
    handler_t & handler_t::get ()
    {
      static handler_t handler;
      return handler;
    }

    void handler_t::start ()
    {
      {
        boost::unique_lock<boost::recursive_mutex> lock;
        if (! m_started)
        {
          sigset_t restrict;
          sigfillset (&restrict);
          //          sigdelset (&restrict, SIGTSTP);
          //          sigdelset (&restrict, SIGCONT);
          pthread_sigmask (SIG_BLOCK, &restrict, 0);

          m_stopping = false;

          DLOG(TRACE, "starting...");

          m_handler_thread = thread_ptr_t
            (new boost::thread (boost::bind ( &handler_t::handler_thread_main
                                            , this
                                            )
                               )
            );
          m_worker_thread = thread_ptr_t
            (new boost::thread (boost::bind ( &handler_t::worker_thread_main
                                            , this
                                            )
                               )
            );

          m_signals_in_progress = 0;
          m_started = true;
        }
      }

      return;
    }

    void handler_t::stop ()
    {
      {
        boost::unique_lock<boost::recursive_mutex> lock (m_mutex);
        if (m_started)
        {
          m_stopping = true;
          this->raise (0);
          ::raise (SIGINT);
          DLOG(TRACE, "stopping...");
        }
      }

      join ();

      {
        boost::unique_lock<boost::recursive_mutex> lock (m_mutex);
        m_started = false;
        m_signal_delivered.notify_all();
      }
    }

    void handler_t::wait ()
    {
      boost::unique_lock<boost::recursive_mutex> lock (m_signal_queue_mutex);
      while (m_started && (!m_signal_queue.empty() || m_signals_in_progress))
      {
        m_signal_delivered.wait (lock);
      }
    }

    void handler_t::join ()
    {
      if (m_worker_thread && (boost::this_thread::get_id() != m_worker_thread->get_id()))
        m_worker_thread->join ();
      if (m_handler_thread)
        m_handler_thread->join ();
    }

    void handler_t::raise (const signal_t sig)
    {
      boost::unique_lock<boost::recursive_mutex> lock (m_signal_queue_mutex);
      m_signal_queue.push_back (sig);
      m_signal_arrived.notify_one ();
    }

    void handler_t::disconnect (const connection_t & con)
    {
      boost::unique_lock<boost::recursive_mutex> lock (m_signal_map_mutex);

      function_list_t & fun_list (m_signal_map[con.m_sig]);
      for ( function_list_t::iterator fun(fun_list.begin())
          ; fun != fun_list.end()
          ; ++fun
          )
      {
        if (fun->id == con.m_id)
        {
          fun->dispose = true;
          break;
        }
      }
    }

    // private implementation
    handler_t::connection_t handler_t::connect_impl ( const signal_t sig
                                                    , signal_handler_function_t f
                                                    )
    {
      boost::unique_lock<boost::recursive_mutex> lock (m_signal_map_mutex);
      connection_id_t next_id (++m_next_connection_id);

      m_signal_map[ sig ].push_back
        (registered_function_t (next_id, f));

      DLOG(TRACE, "signal handler registered for signal " << sig);

      return connection_t (sig, next_id, *this);
    }

    void handler_t::handler_thread_main ()
    {
      char buf[1024];
      sigset_t restrict;

      sigemptyset (&restrict);
      sigaddset (&restrict, SIGALRM);
      sigaddset (&restrict, SIGTSTP);
      sigaddset (&restrict, SIGCONT);
      sigaddset (&restrict, SIGINT);
      sigaddset (&restrict, SIGTERM);
      sigaddset (&restrict, SIGSEGV);
      sigaddset (&restrict, SIGFPE);
      sigaddset (&restrict, SIGBUS);
      sigaddset (&restrict, SIGUSR1);
      sigaddset (&restrict, SIGUSR2);
      sigaddset (&restrict, SIGHUP);
      sigaddset (&restrict, SIGCHLD);

      pthread_sigmask (SIG_BLOCK, &restrict, 0);

      while (!m_stopping)
      {
        boost::this_thread::interruption_point ();

        siginfo_t sig_info;

        struct timespec timeout;
        timeout.tv_sec = 1;
        timeout.tv_nsec = 500 * 1000 * 1000;

        DLOG(TRACE, "handler waiting for signals");

        int ec = sigtimedwait (&restrict, &sig_info, &timeout);
        //int ec = sigwaitinfo (&restrict,&sig_info);

        if (m_stopping) break;

        if (ec >= 0)
        {
          DLOG(TRACE, "got signal: " << show (sig_info));
          this->raise (sig_info.si_signo);
        }
        else if (errno != EAGAIN && errno != EINTR)
        {
          LOG( ERROR
             , "sigwait() returned an error [" << ec << "]: " << strerror_r ( errno
                                                                            , buf
                                                                            , sizeof(buf)
                                                                            )
             );
        }
        else
        {
          DLOG(TRACE, "sigwait() has been interrupted!");
        }
      }

      DLOG(TRACE, "main handler terminating");
    }

    void handler_t::worker_thread_main ()
    {
      while (!m_stopping)
      {
        boost::this_thread::interruption_point ();

        DLOG(TRACE, "worker waiting for signals");

        signal_t sig (next_signal());

        if (m_stopping) break;

        try
        {
          std::size_t count (0);

          DLOG(TRACE, "delivering signal " << sig);
          count = deliver_signal (sig);
          DLOG(TRACE, "delivered signal " << count << " times");

          signal_delivered (sig);
        }
        catch (std::exception const & ex)
        {
          LOG(WARN, "error during signal handling: " << ex.what());
        }
      }

      DLOG(TRACE, "worker terminating");
    }

    handler_t::signal_t handler_t::next_signal ()
    {
      boost::unique_lock<boost::recursive_mutex> lock (m_signal_queue_mutex);
      while (m_signal_queue.empty())
      {
        m_signal_arrived.wait (lock);
      }
      signal_t s (m_signal_queue.front());
      m_signal_queue.pop_front ();
      ++m_signals_in_progress;
      return s;
    }

    void handler_t::signal_delivered (const signal_t &)
    {
      boost::unique_lock<boost::recursive_mutex> lock (m_signal_queue_mutex);
      --m_signals_in_progress;
      m_signal_delivered.notify_all ();
    }

    std::size_t handler_t::deliver_signal (const signal_t & s)
    {
      boost::unique_lock<boost::recursive_mutex> lock (m_signal_map_mutex);

      std::size_t count (0);

      function_list_t & fun_list (m_signal_map[s]);
      for ( function_list_t::iterator fun(fun_list.begin())
          ; fun != fun_list.end()
          ; // manual increment
          )
      {
        if (! fun->dispose)
        {
          fun->fun (s);
          ++fun;
          ++count;
        }
        else
        {
          fun = fun_list.erase (fun);
        }
      }

      return count;
    }
  }
}
