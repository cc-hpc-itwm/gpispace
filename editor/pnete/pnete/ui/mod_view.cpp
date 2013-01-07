// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/mod_view.hpp>

#include <pnete/ui/module_call_widget.hpp>

#include <xml/parse/type/function.hpp>

#include <QStringList>
#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      mod_view::mod_view ( data::proxy::type& proxy
                         , const ::xml::parse::id::ref::module& mod
                         , const data::handle::function& function
                         )
        : document_view (function, proxy, tr ("anonymous module call>>"))
      {
        setWidget (new module_call_widget (mod, function, this));
      }
    }
  }
}
