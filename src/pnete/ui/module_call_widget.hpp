// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/data/handle/module.fwd.hpp>

#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class module_call_widget : public QWidget
      {
        Q_OBJECT

      public:
        module_call_widget ( const data::handle::module&
                           , const data::handle::function&
                           , QWidget* parent = nullptr
                           );
      };
    }
  }
}
