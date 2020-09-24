#pragma once

#include <iml/vmem/gaspi/pc/type/types.hpp>
#include <iml/vmem/gaspi/pc/type/handle.hpp>

#include <atomic>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class handle_generator_t
      {
      public:
        //! Initialize a handle generator which will generate unique
        //! handles with the global part \a identifier.
        explicit handle_generator_t (type::size_t identifier);

        //! Generate the next handle. Thread-safe.
        type::handle_t next();

      private:
        type::size_t m_node_identifier;
        std::atomic<type::size_t> m_counter;
      };
    }
  }
}
