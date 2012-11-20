// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/expression_view.hpp>

#include <QStringList>
#include <QString>

#include <pnete/ui/expression_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      expression_view::expression_view
        ( data::proxy::type& proxy
        , data::proxy::expression_proxy::data_type & expression
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
