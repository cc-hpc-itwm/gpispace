#ifndef GPI_PLUGIN_HPP
#define GPI_PLUGIN_HPP 1

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle.hpp>
#include <gpi-space/pc/type/handle_descriptor.hpp>
#include <gpi-space/pc/type/segment_descriptor.hpp>
#include <gpi-space/pc/type/info_descriptor.hpp>
#include <gpi-space/pc/type/memory_location.hpp>

namespace gpi
{
  class GPI
  {
  public:
    virtual ~GPI() {}

    virtual gpi::pc::type::handle_id_t alloc ( const gpi::pc::type::segment_id_t
                                             , const gpi::pc::type::size_t
                                             , const std::string & desc
                                             , const gpi::pc::type::flags_t = gpi::pc::type::handle::F_NONE
                                             ) = 0;
    virtual void free (const gpi::pc::type::handle_id_t) = 0;
    virtual gpi::pc::type::handle::list_t list_allocations (const gpi::pc::type::segment_id_t seg = gpi::pc::type::segment::SEG_INVAL) = 0;

    virtual gpi::pc::type::queue_id_t memcpy ( gpi::pc::type::memory_location_t const & dst
                                             , gpi::pc::type::memory_location_t const & src
                                             , const gpi::pc::type::size_t amount
                                             , const gpi::pc::type::queue_id_t queue
                                             ) = 0;

    virtual gpi::pc::type::handle_t memset (const gpi::pc::type::handle_t h
                                           , int value
                                           , size_t count
                                           ) = 0;

    virtual void * ptr(const gpi::pc::type::handle_t h) = 0;
    virtual gpi::pc::type::size_t wait (const gpi::pc::type::queue_id_t) = 0;
    virtual std::vector<gpi::pc::type::size_t> wait () = 0;

    virtual gpi::pc::type::segment_id_t register_segment( std::string const & name
                                                        , const gpi::pc::type::size_t sz
                                                        , const gpi::pc::type::flags_t = 0
                                                        ) = 0;
    virtual void unregister_segment(const gpi::pc::type::segment_id_t) = 0;
    virtual void attach_segment(const gpi::pc::type::segment_id_t id) = 0;
    virtual void detach_segment(const gpi::pc::type::segment_id_t id) = 0;
    virtual gpi::pc::type::segment::list_t list_segments () = 0;
    virtual gpi::pc::type::info::descriptor_t collect_info () = 0;

    virtual bool is_connected () const = 0;
    virtual bool connect () = 0;
    virtual void garbage_collect () = 0;
  };
}

#endif
