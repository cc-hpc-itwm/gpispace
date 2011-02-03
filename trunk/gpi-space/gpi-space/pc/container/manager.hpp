#ifndef GPI_SPACE_PC_CONTAINER_MANAGER_HPP
#define GPI_SPACE_PC_CONTAINER_MANAGER_HPP 1

#include <gpi-space/pc/container/connector.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      class manager_t
      {
      public:
        typedef gpi::pc::container::process_t<manager_t> process_type;
        typedef gpi::pc::container::connector_t< manager_t
                                               , process_type
                                               > connector_type;

        explicit
        manager_t (std::string const & p)
          : m_connector (*this, p)
        {}

        ~manager_t ();

        void start();
        void stop ();
      private:
        connector_type m_connector;
      };
    }
  }
}

#include "manager.ipp"

#endif
