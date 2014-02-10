#include "factory.hpp"

#include <gpi-space/pc/memory/gpi_area.hpp>
#include <gpi-space/pc/memory/sfs_area.hpp>
#include <gpi-space/pc/memory/shm_area.hpp>

#include <gpi-space/pc/global/topology.hpp>

#include <fhglog/fhglog.hpp>

#include <fhg/util/url.hpp>
#include <fhg/util/url_io.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      factory_t::factory_t()
      {
        m_factory_functions.insert
          (std::make_pair ( "gpi"
                          , boost::bind ( gpi_area_t::create
                                        , _1
                                        , boost::ref (global::topology ())
                                        )
                          )
          );
        m_factory_functions.insert
          (std::make_pair ( "shm"
                          , &shm_area_t::create
                          )
          );
        m_factory_functions.insert
          (std::make_pair ( "sfs"
                          , boost::bind ( sfs_area_t::create
                                        , _1
                                        , boost::ref (global::topology ())
                                        )
                          )
          );
      }

      area_ptr_t
      factory_t::create (std::string const &url_s)
      {
        fhg::util::url_t url (url_s);
        function_map_t::iterator fun_it = m_factory_functions.find (url.type ());
        if (fun_it != m_factory_functions.end ())
        {
          MLOG (DEBUG, "creating '" << url.type () << "' segment: " << url);
          return fun_it->second (url_s);
        }
        else
        {
          throw std::runtime_error
            ("no memory type registered with: '" + url.type () + "'");
        }
      }

      factory_t & factory ()
      {
        static factory_t instance;
        return instance;
      }
    }
  }
}
