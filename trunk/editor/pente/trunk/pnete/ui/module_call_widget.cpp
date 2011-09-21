// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <QWidget>

#include <pnete/ui/module_call_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      module_call_widget::module_call_widget
        ( data::proxy::type& proxy
        , data::proxy::mod_proxy::data_type& mod
        , QWidget* parent
        )
          : base_editor_widget (proxy, parent)
          , _mod (mod)
          , _port_list (mod.in(), mod.out(), this)
      {}
    }
  }
}
