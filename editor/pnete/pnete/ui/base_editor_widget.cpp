// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/base_editor_widget.hpp>

#include <pnete/data/internal.hpp>
#include <pnete/data/change_manager.hpp>

#include <xml/parse/types.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      base_editor_widget::base_editor_widget
        ( data::proxy::type& proxy
        , QWidget* parent
        )
          : QWidget (parent)
          , _proxy (proxy)
      {}

      data::proxy::type& base_editor_widget::proxy () const { return _proxy; }
      data::internal_type* base_editor_widget::root () const
      {
        return data::proxy::root (proxy());
      }
      data::change_manager_t& base_editor_widget::change_manager () const
      {
        return root()->change_manager();
      }
      ::xml::parse::type::function_type& base_editor_widget::function () const
      {
        return data::proxy::function (proxy());
      }
    }
  }
}
