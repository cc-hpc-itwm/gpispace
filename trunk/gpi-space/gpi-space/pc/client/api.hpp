#ifndef GPI_SPACE_PC_CLIENT_API_HPP
#define GPI_SPACE_PC_CLIENT_API_HPP 1

#include <string>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include <gpi-space/signal_handler.hpp>
#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace client
    {
      class api_t : public boost::noncopyable
      {
      public:
        explicit
        api_t (std::string const & path);

        ~api_t ();

        void start ();
        void stop ();

        bool is_connected () const;
        void path (std::string const & p);
        std::string const & path () const;

        // api to gpi
        int write (const void *buf, size_t sz);
        int read (void *buf, size_t sz);

        type::handle_id_t alloc ( const type::segment_id_t
                                , const type::size_t
                                , const type::mode_t = 0777
                                );

        void free (const type::handle_id_t);
      private:
        int connection_lost (int);

        std::string m_path;
        int m_socket;
        bool m_connected;
        gpi::signal::handler_t::connection_t m_signal_connection;
      };
    }
  }
}

#endif
