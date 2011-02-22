#ifndef GPI_SPACE_PC_SEGMENT_MANAGER_HPP
#define GPI_SPACE_PC_SEGMENT_MANAGER_HPP 1

#include <string>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <boost/signals2.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/segment/segment.hpp>

namespace gpi
{
  namespace pc
  {
    namespace segment
    {
      class manager_t : boost::noncopyable
      {
      public:
        typedef boost::shared_ptr<segment_t> segment_ptr;

        // signals
        typedef boost::signals2::signal
            <void (const gpi::pc::type::segment_id_t)>
               segment_signal_t;
        segment_signal_t segment_added;
        segment_signal_t segment_removed;

        manager_t ();
        ~manager_t ();

        void clear ();

        gpi::pc::type::segment_id_t register_segment( const gpi::pc::type::process_id_t creator
                                                    , const std::string & name
                                                    , const gpi::pc::type::size_t size
                                                    , const gpi::pc::type::flags_t flags
                                                    );
        void unregister_segment (const gpi::pc::type::segment_id_t);
        void get_listing (gpi::pc::type::segment::list_t &) const;

        gpi::pc::type::size_t increment_refcount (const gpi::pc::type::segment_id_t seg);
        gpi::pc::type::size_t decrement_refcount (const gpi::pc::type::segment_id_t seg);

        void add_special_segment ( std::string const & name
                                 , const gpi::pc::type::segment_id_t id
                                 , const gpi::pc::type::size_t size
                                 , void *ptr
                                 );
        segment_ptr get_segment (const gpi::pc::type::segment_id_t);
        segment_ptr operator [] (const gpi::pc::type::segment_id_t seg_id)
        {
          return get_segment (seg_id);
        }
      private:
        typedef boost::unordered_map<gpi::pc::type::segment_id_t, segment_ptr> segment_map_t;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        mutable mutex_type m_mutex;
        gpi::pc::type::segment_id_t m_segment_id;
        segment_map_t m_segments;
      };
    }
  }
}

#endif
