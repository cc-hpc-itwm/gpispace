#include "factory.hpp"

#include <gpi-space/pc/memory/gpi_area.hpp>
#include <gpi-space/pc/memory/sfs_area.hpp>
#include <gpi-space/pc/memory/shm_area.hpp>

#include <gpi-space/pc/global/topology.hpp>

#include <fhg/util/url.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      area_ptr_t
      factory_t::create (std::string const &url_s)
      {
        fhg::util::url_t url (url_s);

        return
          ( url.type() == "gpi" ? gpi_area_t::create (url_s, boost::ref (global::topology()))
          : url.type() == "sfs" ? sfs_area_t::create (url_s, boost::ref (global::topology()))
          : url.type() == "shm" ? shm_area_t::create (url_s)
          : throw std::runtime_error
              ("no memory type registered with: '" + url_s + "'")
          );
      }

      factory_t & factory ()
      {
        static factory_t instance;
        return instance;
      }
    }
  }
}
