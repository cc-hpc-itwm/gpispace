// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <pnete/data/handle/expression.fwd.hpp>

#include <pnete/data/handle/meta_base.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/expression.fwd.hpp>

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
        typedef meta_base < ::xml::parse::id::ref::expression
                          , ::xml::parse::type::expression_type
                          > expression_meta_base;
        class expression : public expression_meta_base
        {
        public:
          expression (const expression_meta_base::id_type&, internal_type*);

          void set_content (const QString&);
          QString content() const;

          using expression_meta_base::operator==;
        };
      }
    }
  }
}
