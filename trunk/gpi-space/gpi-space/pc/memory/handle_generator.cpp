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
                                       , const gpi::pc::type::segment::segment_type type
                                       , const gpi::pc::type::size_t counter
                                       )
        {
          gpi::pc::type::handle_t hdl;

          if (gpi::pc::type::segment::SEG_INVAL == type)
          {
            hdl = 0;
          }
          else if (gpi::pc::type::segment::SEG_GPI == type)
          {
            hdl.type = gpi::pc::type::segment::SEG_GPI;

            gpi::pc::type::check_for_overflow<gpi::pc::type::handle_t::ident_bits>(node);
            hdl.gpi.ident = node;

            gpi::pc::type::check_for_overflow<gpi::pc::type::handle_t::global_count_bits>(counter);
            hdl.gpi.cntr = counter;
          }
          else
          {
            hdl.type = type;

            gpi::pc::type::check_for_overflow<gpi::pc::type::handle_t::local_count_bits>(counter);
            hdl.shm.cntr = counter;
          }

          return hdl;
        }
      }

      boost::shared_ptr<handle_generator_t> handle_generator_t::instance;

      handle_generator_t::handle_generator_t(const gpi::pc::type::size_t identifier)
        : m_node_identifier (identifier)
      {
        for (size_t i = 0; i < (1<< gpi::pc::type::handle_t::typec_bits); ++i)
        {
          m_counter.push_back(counter_ptr(new gpi::pc::type::counter_t()));
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

      gpi::pc::type::handle_t
      handle_generator_t::next (const gpi::pc::type::segment::segment_type seg)
      {
        if (gpi::pc::type::segment::SEG_INVAL == seg)
          return gpi::pc::type::handle_t(0);
        assert (seg > 0);
        if (size_t(seg) >= m_counter.size())
          throw std::invalid_argument ("invalid segment type");

        return detail::encode ( m_node_identifier
                              , seg
                              , m_counter[seg]->inc()
                              );
      }
    }
  }
}
