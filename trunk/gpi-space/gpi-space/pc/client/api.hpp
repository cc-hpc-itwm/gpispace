#ifndef GPI_SPACE_PC_CLIENT_API_HPP
#define GPI_SPACE_PC_CLIENT_API_HPP 1

#include <map>
#include <set>
#include <string>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include <gpi-space/signal_handler.hpp>
#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/segment/segment.hpp>

namespace gpi
{
  namespace pc
  {
    namespace client
    {
      class api_t : public boost::noncopyable
      {
      public:
        typedef boost::shared_ptr<gpi::pc::segment::segment_t> segment_ptr;
        typedef std::map<gpi::pc::type::segment_id_t, segment_ptr> segment_map_t;
        typedef std::set<segment_ptr> segment_set_t;

        explicit
        api_t (std::string const & path);

        ~api_t ();

        void start ();
        void stop ();

        bool is_connected () const;
        void path (std::string const & p);
        std::string const & path () const;

        // api to gpi
        type::handle_id_t alloc ( const type::segment_id_t
                                , const type::size_t
                                , const std::string & desc
                                , const type::flags_t = gpi::pc::type::handle::F_NONE
                                );
        void free (const type::handle_id_t);
        gpi::pc::type::handle::list_t list_allocations (const gpi::pc::type::segment_id_t seg = gpi::pc::type::segment::SEG_INVAL);

        gpi::pc::type::queue_id_t
        memcpy ( gpi::pc::type::memory_location_t const & dst
               , gpi::pc::type::memory_location_t const & src
               , const gpi::pc::type::size_t amount
               , const gpi::pc::type::queue_id_t queue
               );

        gpi::pc::type::size_t
        wait (const gpi::pc::type::queue_id_t);

        gpi::pc::type::segment_id_t register_segment( std::string const & name
                                                    , const gpi::pc::type::size_t sz
                                                    , const gpi::pc::type::flags_t = 0
                                                    );
        void unregister_segment(const gpi::pc::type::segment_id_t);
        void attach_segment(const gpi::pc::type::segment_id_t id);
        void detach_segment(const gpi::pc::type::segment_id_t id);
        gpi::pc::type::segment::list_t list_segments ();

        gpi::pc::type::info::descriptor_t collect_info ();
        bool ping ();

        bool is_attached (const gpi::pc::type::segment_id_t id);
        segment_map_t const &  segments () const;
        segment_set_t const & garbage_segments () const;
        void garbage_collect ();
      private:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        int connection_lost (int);
        gpi::pc::proto::message_t communicate (gpi::pc::proto::message_t const &);
        int write (const void * buf, size_t sz);
        int read (void * buf, size_t sz);

        mutable mutex_type m_mutex;
        std::string m_path;
        int m_socket;
        bool m_connected;
        gpi::signal::handler_t::connection_t m_sigpipe_connection;
        gpi::signal::handler_t::connection_t m_sigint_connection;
        segment_map_t m_segments;
        segment_set_t m_garbage_segments;
      };
    }
  }
}

#endif
