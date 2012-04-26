// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_DOCUMENT_VIEW_HPP
#define _PNETE_UI_DOCUMENT_VIEW_HPP 1

#include <QObject>

#include <pnete/data/proxy.hpp>

#include <pnete/ui/dock_widget.hpp>

#include <fhg/util/maybe.hpp>

class QString;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class base_editor_widget;

      class document_view : public dock_widget
      {
        Q_OBJECT;

      public:
        document_view (const data::proxy::type& proxy);

        base_editor_widget* widget() const;
        void setWidget (base_editor_widget* widget);

      protected:
        void set_title (const fhg::util::maybe<std::string>&);

      signals:
        void focus_gained (QWidget*);

      public slots:
        void slot_set_function_name ( const QObject*
                                    , const ::xml::parse::type::function_type&
                                    , const QString&
                                    );
      private slots:
        void visibility_changed (bool);

      private:
        virtual QString fallback_title() const = 0;
      };
    }
  }
}

#endif
