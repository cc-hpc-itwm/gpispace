#include "factory.hpp"

#include <fhglog/fhglog.hpp>

#include <fhg/util/url.hpp>
#include <fhg/util/url_io.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      void
      factory_t::register_type ( std::string const &typ
                               , factory_function_t fun
                               )
      {
        m_factory_functions [typ] = fun;
      }

      void
      factory_t::unregister_type (std::string const &typ)
      {
        m_factory_functions.erase (typ);
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
