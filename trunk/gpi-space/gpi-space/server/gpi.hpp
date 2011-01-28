#ifndef GPI_SPACE_SERVER_HPP
#define GPI_SPACE_SERVER_HPP 1

#include <gpi-space/types.hpp>
#include <gpi-space/error.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

namespace gpi
{
  namespace server
  {
    namespace exception
    {
      struct gpi_error : public std::runtime_error
      {
        explicit
        gpi_error (gpi::error::code_t const & ec)
          : std::runtime_error (ec.name())
          , value (ec.value())
          , user_message ()
          , message ("gpi::error[" + boost::lexical_cast<std::string>(value) + "]: " + ec.name())
        {}

        explicit
        gpi_error (gpi::error::code_t const & ec, std::string const & m)
          : std::runtime_error (ec.name ())
          , value (ec.value())
          , user_message (m)
          , message ("gpi::error[" + boost::lexical_cast<std::string>(value) + "]: " + ec.name() + ": " + user_message)
        {}

        virtual ~gpi_error () throw () {}

        virtual const char * what () const throw ()
        {
          return message.c_str();
        }

        int value;
        const std::string user_message;
        const std::string message;
      };
    }

    namespace real
    {
      class gpi_api_t
      {
      public:
        gpi_api_t ();
        ~gpi_api_t();

        void init (int ac, char *av[]);

        void set_memory_size (const gpi::size_t);

        // wrapped C function calls
        void start (const gpi::timeout_t timeout);
        void stop ();
        void kill ();

        gpi::size_t number_of_counters () const;
        gpi::size_t number_of_queues () const;
        gpi::size_t queue_depth () const;
        gpi::version_t version () const;
        gpi::port_t port () const;
        gpi::size_t number_of_nodes () const;
        std::string hostname (const gpi::rank_t) const;
        gpi::rank_t rank () const;
        void * dma_ptr (void);

        void set_network_type (const gpi::network_type_t);
        void set_port (const gpi::port_t);
        void set_mtu (const gpi::size_t);
        void set_number_of_processes (const gpi::size_t);

        bool ping (const gpi::rank_t) const;
        bool ping (std::string const & hostname) const;
        bool is_master (void) const;
        bool is_slave (void) const;

        void barrier (void) const;

        void read_dma ( const offset_t local_offset
                      , const offset_t remote_offset
                      , const size_t amount
                      , const rank_t from_node
                      , const queue_desc_t queue
                      );
        void write_dma ( const offset_t local_offset
                       , const offset_t remote_offset
                       , const size_t amount
                       , const rank_t to_node
                       , const queue_desc_t queue
                       );

        void send_dma ( const offset_t local_offset
                      , const size_t amount
                      , const rank_t to_node
                      , const queue_desc_t queue
                      );
        void recv_dma ( const offset_t local_offset
                      , const size_t size
                      , const rank_t from_node
                      , const queue_desc_t queue
                      );

        size_t wait_dma (const queue_desc_t queue);

        void send_passive ( const offset_t local_offset
                          , const size_t size
                          , const rank_t to_rank
                          );
        void recv_passive ( const offset_t local_offset
                          , const size_t size
                          , rank_t & from_rank
                          );
        void wait_passive ( void );

      private:
        void handle_signal (int);
        void signal_handler (void); // thread entry

        int   m_ac;
        char **m_av;
        bool  m_startup_done;
        mutable rank_t m_rank;
        mutable size_t m_num_nodes;
        mutable size_t m_mem_size;
        boost::shared_ptr<boost::thread> m_signal_handler;
      };
    }
  }
}

#endif
