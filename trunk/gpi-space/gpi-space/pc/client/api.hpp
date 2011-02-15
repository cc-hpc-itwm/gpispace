#ifndef GPI_SPACE_PC_CLIENT_API_HPP
#define GPI_SPACE_PC_CLIENT_API_HPP 1

#include <string>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include <gpi-space/signal_handler.hpp>
#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/proto/message.hpp>

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
                                , const std::string & desc
                                , const type::flags_t = 0777
                                );
        void free (const type::handle_id_t);
        gpi::pc::type::handle::list_t list_allocations (const gpi::pc::type::segment_id_t seg = gpi::pc::type::segment::SEG_INVAL);

        gpi::pc::type::segment_id_t register_segment( std::string const & name
                                                    , const gpi::pc::type::size_t sz
                                                    , const gpi::pc::type::flags_t = 0
                                                    );
        void unregister_segment(const gpi::pc::type::segment_id_t);
        void attach_segment(const gpi::pc::type::segment_id_t id);
        void detach_segment(const gpi::pc::type::segment_id_t id);
        gpi::pc::type::segment::list_t list_segments ();
      private:
        int connection_lost (int);
        gpi::pc::proto::message_t communicate (gpi::pc::proto::message_t const &);

        std::string m_path;
        int m_socket;
        bool m_connected;
        gpi::signal::handler_t::connection_t m_sigpipe_connection;
        gpi::signal::handler_t::connection_t m_sigint_connection;
      };
    }
  }
}

#endif
