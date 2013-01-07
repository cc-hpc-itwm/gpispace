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
        , const data::handle::function& function
        )
          : document_view (function, proxy, tr ("<<anonymous expression>>"))
      {
        //! \todo submit known types
        setWidget (new expression_widget ( proxy
                                         , expression
                                         , function
                                         , QStringList()
                                         , this
                                         )
                  );
        set_title (function.get().name());
      }
    }
  }
}
