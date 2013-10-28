#ifndef GPI_SPACE_SIGNAL_HANDLER_HPP
#define GPI_SPACE_SIGNAL_HANDLER_HPP 1

#include <deque>

#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

namespace gpi
{
  namespace signal
  {
    class handler_t
    {
      typedef boost::shared_ptr<boost::thread> thread_ptr_t;
    public:
      typedef int signal_t;
      typedef uint64_t connection_id_t;
      typedef boost::function<void (signal_t)> signal_handler_function_t;

    private:
      struct registered_function_t
      {
        registered_function_t ( const connection_id_t an_id
                              , signal_handler_function_t a_fun
                              , bool one_shot = false
                              )
          : id (an_id)
          , fun (a_fun)
          , dispose (false)
          , is_one_shot (one_shot)
        {}

        void operator () (int s)
        {
          if (is_one_shot)
          {
            dispose = true;
          }
          fun(s);
        }

        connection_id_t id;
        signal_handler_function_t fun;
        bool dispose;
        bool is_one_shot;
      };
    public:
      typedef std::vector<registered_function_t> function_list_t;
      typedef boost::unordered_map <signal_t, function_list_t> signal_map_t;
      typedef std::deque<signal_t> signal_queue_t;

      struct connection_t
      {
        connection_t ()
          : m_sig (-1)
          , m_id (0)
          , m_hdl (0)
        {}

        connection_t (const signal_t sig, connection_id_t id, handler_t & h)
          : m_sig (sig)
          , m_id (id)
          , m_hdl (&h)
        {}

        connection_t (const connection_t & other)
          : m_sig (other.m_sig)
          , m_id (other.m_id)
          , m_hdl (other.m_hdl)
        {}

        connection_t & operator= (const connection_t & other)
        {
          m_sig = other.m_sig;
          m_id = other.m_id;
          m_hdl =other.m_hdl;
          return *this;
        }

        void disconnect ()
        {
          if (m_hdl)
            m_hdl->disconnect (*this);
        }

        signal_t m_sig;
        connection_id_t m_id;
        handler_t * m_hdl;
      };

      struct scoped_connection_t : public connection_t, boost::noncopyable
      {
        explicit
        scoped_connection_t (const connection_t & c)
          : connection_t (c)
        {}

        ~scoped_connection_t ()
        {
          disconnect ();
        }
      };

      static handler_t & get ();

      ~handler_t ();

      void start();
      void stop();
      void wait ();
      void join ();
      void raise (const signal_t);

      template <typename Fun>
      connection_t connect(const signal_t s, Fun f)
      {
        return connect_impl(s, f);
      }

      void disconnect (const connection_t & c);
    private:
      handler_t();
      handler_t(const handler_t & h);

      connection_t connect_impl( const signal_t
                               , signal_handler_function_t
                               );

      void handler_thread_main ();
      void worker_thread_main ();

      signal_t next_signal ();
      std::size_t deliver_signal (const signal_t & s);
      void signal_delivered (const signal_t & s);

      connection_id_t m_next_connection_id;
      std::size_t m_signals_in_progress;
      boost::recursive_mutex m_signal_queue_mutex;
      boost::recursive_mutex m_signal_map_mutex;
      boost::recursive_mutex m_mutex;
      boost::condition_variable_any m_signal_arrived;
      boost::condition_variable_any m_signal_delivered;
      boost::condition_variable_any m_stopped;
      signal_queue_t m_signal_queue;
      signal_map_t m_signal_map;
      thread_ptr_t m_handler_thread;
      thread_ptr_t m_worker_thread;

      bool m_stopping;
      bool m_started;
    };

    inline handler_t & handler ()
    {
      return handler_t::get();
    }
  }
}

#endif
