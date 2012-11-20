// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/expression_view.hpp>

#include <pnete/ui/expression_widget.hpp>

#include <xml/parse/type/function.hpp>

#include <QStringList>
#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      expression_view::expression_view
        ( data::proxy::type& proxy
        , const data::handle::expression& expression
        )
          : document_view (proxy)
      {
        //! \todo submit known types
        setWidget (new expression_widget ( proxy
                                         , expression
                                         , QStringList()
                                         , this
                                         )
                  );
        set_title (data::proxy::function (proxy).get().name());
      }
      QString expression_view::fallback_title () const
      {
        return tr("<<anonymous expression>>");
      }
    }
  }
}
