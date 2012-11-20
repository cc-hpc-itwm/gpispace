// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_BASE_EDITOR_WIDGET_HPP
#define _PNETE_UI_BASE_EDITOR_WIDGET_HPP 1

#include <QObject>
#include <QWidget>

#include <pnete/data/proxy.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class internal_type;
      class change_manager_t;
    }

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
        ::xml::parse::id::ref::function function() const;

      signals:
        void focus_gained (QWidget*);

      private:
        data::proxy::type& _proxy;
      };
    }
  }
}

#endif
