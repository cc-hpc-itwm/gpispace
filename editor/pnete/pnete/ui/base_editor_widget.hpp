// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef PNETE_UI_BASE_EDITOR_WIDGET_HPP
#define PNETE_UI_BASE_EDITOR_WIDGET_HPP

#include <pnete/ui/base_editor_widget.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/data/internal.fwd.hpp>
#include <pnete/data/proxy.hpp>

#include <QObject>
#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class base_editor_widget : public QWidget
      {
        Q_OBJECT;

      public:
        base_editor_widget ( data::proxy::type& proxy
                           , QWidget* parent = NULL
                           );

        data::proxy::type& proxy () const;
        data::change_manager_t& change_manager () const;
        data::internal_type* root () const;
        data::handle::function function() const;

      private:
        data::proxy::type& _proxy;
      };
    }
  }
}

#endif
