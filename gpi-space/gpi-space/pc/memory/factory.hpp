#ifndef GPI_SPACE_PC_MEMORY_FACTORY_HPP
#define GPI_SPACE_PC_MEMORY_FACTORY_HPP

#include <string>

#include <boost/shared_ptr.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class area_t;
      typedef boost::shared_ptr<area_t> area_ptr_t;

      class factory_t
      {
      public:
        area_ptr_t create (std::string const & url);
      };

      factory_t & factory ();
    }
  }
}

#endif
