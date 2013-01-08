// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_MODULE_CALL_WIDGET_HPP
#define _PNETE_UI_MODULE_CALL_WIDGET_HPP 1

#include <pnete/data/handle/function.fwd.hpp>

#include <xml/parse/id/types.fwd.hpp>

#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class module_call_widget : public QWidget
      {
        Q_OBJECT;

      public:
        module_call_widget ( const ::xml::parse::id::ref::module&
                           , const data::handle::function&
                           , QWidget* parent = NULL
                           );
      };
    }
  }
}

#endif
