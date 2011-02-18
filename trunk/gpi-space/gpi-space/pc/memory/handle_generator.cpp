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

      boost::shared_ptr<handle_generator_t> handle_generator_t::instance;

      handle_generator_t::handle_generator_t(const gpi::pc::type::size_t identifier)
        : m_node_identifier (identifier)
      {
        for (int i (1); i <= gpi::pc::type::segment::SEG_SHARED; ++i)
        {
          m_counter[i] = counter_ptr(new gpi::pc::type::counter_t());
        }
      }

      void handle_generator_t::create (const gpi::pc::type::size_t identifier)
      {
        assert (! instance);
        instance.reset (new handle_generator_t (identifier));
      }

      handle_generator_t & handle_generator_t::get ()
      {
        assert (instance);
        return *instance;
      }

      void handle_generator_t::destroy ()
      {
        if (instance)
        {
          instance.reset ();
        }
      }

      gpi::pc::type::handle_t handle_generator_t::next (const gpi::pc::type::segment_id_t seg_id)
      {
        if (! gpi::pc::type::segment::traits::is_valid(seg_id))
        {
          return gpi::pc::type::handle_t(0);
        }
        else
        {
          gpi::pc::type::size_t count
            (m_counter[seg_id & 0x3]->inc());
          return detail::encode (m_node_identifier, seg_id, count);
        }
      }
    }
  }
}
