#include "handle_generator.hpp"

#include <gpi-space/pc/type/segment_descriptor.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace detail
      {
        gpi::pc::type::handle_t encode ( const gpi::pc::type::size_t node
                                       , const gpi::pc::type::segment_id_t seg
                                       , const gpi::pc::type::size_t counter
                                       )
        {
          gpi::pc::type::handle_t hdl;

          if (gpi::pc::type::segment::SEG_INVAL == seg)
          {
            hdl = 0;
          }
          else if (gpi::pc::type::segment::SEG_GLOBAL == seg)
          {
            hdl.type = gpi::pc::type::segment::SEG_GLOBAL;

            gpi::pc::type::check_for_overflow<gpi::pc::type::handle_t::ident_bits>(node);
            hdl.global.ident = node;

            gpi::pc::type::check_for_overflow<gpi::pc::type::handle_t::global_count_bits>(counter);
            hdl.global.cntr = counter;
          }
          else
          {
            if (gpi::pc::type::segment::SEG_LOCAL == seg)
              hdl.type = gpi::pc::type::segment::SEG_LOCAL;
            else
              hdl.type = gpi::pc::type::segment::SEG_SHARED;

            gpi::pc::type::check_for_overflow<gpi::pc::type::handle_t::local_count_bits>(counter);
            hdl.local.cntr = counter;
          }

          return hdl;
        }
      }

      handle_generator_t * handle_generator_t::instance = 0;

      handle_generator_t::handle_generator_t(const gpi::pc::type::size_t identifier)
        : m_node_identifier (identifier)
        , m_counter (0)
      {}

      void handle_generator_t::create (const gpi::pc::type::size_t identifier)
      {
        assert (instance == 0);
        instance = new handle_generator_t (identifier);
      }

      handle_generator_t & handle_generator_t::get ()
      {
        assert (instance != 0);
        return *instance;
      }

      void handle_generator_t::destroy ()
      {
        if (instance)
        {
          delete instance;
          instance = 0;
        }
      }

      gpi::pc::type::size_t handle_generator_t::increment ()
      {
        lock_type lock (m_mutex);
        gpi::pc::type::size_t new_count (m_counter + 1);
        if (! new_count) // TODO: branch hint unlikely
        {
          throw std::runtime_error ("cannot create new handle: would overflow");
        }
        m_counter = new_count;
        return new_count;
      }

      gpi::pc::type::handle_t handle_generator_t::next (const gpi::pc::type::segment_id_t seg_id)
      {
        if (! gpi::pc::type::segment::traits::is_valid(seg_id))
        {
          return gpi::pc::type::handle_t(0);
        }
        else
        {
          gpi::pc::type::size_t counter (increment());
          return detail::encode (m_node_identifier, seg_id, counter);
        }
      }
    }
  }
}
