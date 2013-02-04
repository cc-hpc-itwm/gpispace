// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/expression.hpp>

#include <pnete/data/change_manager.hpp>

#include <xml/parse/type/expression.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        expression::expression ( const expression_meta_base::id_type& id
                               , internal_type* document
                               )
          : expression_meta_base (id, document)
        { }

        void expression::set_content (const QString& content)
        {
          change_manager().set_expression (*this, content);
        }
        QString expression::content() const
        {
          return QString::fromStdString (get().expression ("\n"));
        }
      }
    }
  }
}
