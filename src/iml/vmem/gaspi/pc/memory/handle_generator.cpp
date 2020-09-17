#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <iml/util/assert.hpp>

#include <iml/vmem/gaspi/pc/type/segment_descriptor.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace detail
      {
        gpi::pc::type::handle_t encode ( const gpi::pc::type::size_t node
                                       , const gpi::pc::type::size_t counter
                                       )
        {
          gpi::pc::type::handle_t hdl;

            gpi::pc::type::check_for_overflow<gpi::pc::type::handle_t::ident_bits>(node);
            hdl.ident = node;

            gpi::pc::type::check_for_overflow<gpi::pc::type::handle_t::global_count_bits>(counter);
            hdl.cntr = counter;

          return hdl;
        }
      }

      handle_generator_t::handle_generator_t(const gpi::pc::type::size_t identifier)
        : m_node_identifier (identifier)
        , m_counter (0)
      {}

      gpi::pc::type::handle_t handle_generator_t::next()
      {
        return detail::encode ( m_node_identifier
                              , ++m_counter
                              );
      }
    }
  }
}
