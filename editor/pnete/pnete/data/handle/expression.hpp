// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_DATA_HANDLE_EXPRESSION_HPP
#define FHG_PNETE_DATA_HANDLE_EXPRESSION_HPP

#include <pnete/data/handle/expression.fwd.hpp>

#include <pnete/data/handle/meta_base.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/expression.fwd.hpp>

class QObject;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        typedef meta_base < ::xml::parse::id::ref::expression
                          , ::xml::parse::type::expression_type
                          > expression_meta_base;
        class expression : public expression_meta_base
        {
        public:
          expression ( const expression_meta_base::id_type& id
                     , change_manager_t& change_manager
                     );

          using expression_meta_base::operator==;
        };
      }
    }
  }
}

#endif
