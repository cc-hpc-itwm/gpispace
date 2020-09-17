#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      handle_generator_t::handle_generator_t (type::size_t identifier)
        : m_node_identifier (identifier)
        , m_counter (0)
      {}

      gpi::pc::type::handle_t handle_generator_t::next()
      {
        return {m_node_identifier, ++m_counter};
      }
    }
  }
}
