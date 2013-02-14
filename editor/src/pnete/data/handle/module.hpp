// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_DATA_HANDLE_MODULE_HPP
#define FHG_PNETE_DATA_HANDLE_MODULE_HPP

#include <pnete/data/handle/module.fwd.hpp>

#include <pnete/data/handle/meta_base.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/mod.fwd.hpp>

class QObject;
class QString;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        typedef meta_base < ::xml::parse::id::ref::module
                          , ::xml::parse::type::module_type
                          > module_meta_base;
        class module : public module_meta_base
        {
        public:
          module (const module_meta_base::id_type&, internal_type*);

          using module_meta_base::operator==;
        };
      }
    }
  }
}

#endif
