#ifndef GPI_SPACE_PC_CLIENT_API_HPP
#define GPI_SPACE_PC_CLIENT_API_HPP 1

#include <fhg/syscall.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/type/typedefs.hpp>

#include <we/type/range.hpp>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <map>
#include <set>
#include <list>
#include <string>

namespace gpi
{
  namespace pc
  {
    namespace client
    {
      struct transfer_t
      {
        transfer_t ( gpi::pc::type::memory_location_t const& src
                   , gpi::pc::type::memory_location_t const& dst
                   , std::size_t amount
                   )
          : src (src)
          , dst (dst)
          , amount (amount)
        {}

        const gpi::pc::type::memory_location_t src;
        const gpi::pc::type::memory_location_t dst;
        const std::size_t amount;
      };

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
                                , const type::flags_t = gpi::pc::F_NONE
                                );
        void free (const type::handle_id_t);

        gpi::pc::type::handle::descriptor_t
        info(const gpi::pc::type::handle_t h);

        gpi::pc::type::handle::list_t
        list_allocations (const gpi::pc::type::segment_id_t seg = gpi::pc::type::segment::SEG_INVAL);

        gpi::pc::type::queue_id_t
        memcpy ( gpi::pc::type::memory_location_t const & dst
               , gpi::pc::type::memory_location_t const & src
               , const gpi::pc::type::size_t amount
               , const gpi::pc::type::queue_id_t queue
               );

        gpi::pc::type::handle_t
        memset (const gpi::pc::type::handle_t h, int value, size_t count);

        std::function<double (std::string const&)>
        transfer_costs (std::list<std::pair<we::local::range, we::global::range>> const&);

        void * ptr(const gpi::pc::type::handle_t h);

        gpi::pc::type::size_t
        wait (const gpi::pc::type::queue_id_t);

        std::vector<gpi::pc::type::size_t>
        wait ();

        gpi::pc::type::segment_id_t register_segment( std::string const & name
                                                    , const gpi::pc::type::size_t sz
                                                    , const gpi::pc::type::flags_t = 0
                                                    );
        void unregister_segment(const gpi::pc::type::segment_id_t);
        void attach_segment(const gpi::pc::type::segment_id_t id);
        void detach_segment(const gpi::pc::type::segment_id_t id);
        gpi::pc::type::segment::list_t list_segments ();

        gpi::pc::type::segment_id_t add_memory (const std::string & url);
        void del_memory (gpi::pc::type::segment_id_t);

        gpi::pc::type::info::descriptor_t collect_info () const;
        bool ping ();

        bool is_attached (const gpi::pc::type::segment_id_t id);
        segment_map_t const &  segments () const;
        segment_map_t &  segments ();
        segment_set_t const & garbage_segments () const;
        void garbage_collect ();
      private:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        gpi::pc::proto::message_t communicate (gpi::pc::proto::message_t const &);
        ssize_t write (const void * buf, size_t sz);
        ssize_t read (void * buf, size_t sz);

        mutable mutex_type m_mutex;
        std::string m_path;
        int m_socket;
        segment_map_t m_segments;
        segment_set_t m_garbage_segments;
        gpi::pc::type::info::descriptor_t m_info;
      };
    }
  }
}

#endif
