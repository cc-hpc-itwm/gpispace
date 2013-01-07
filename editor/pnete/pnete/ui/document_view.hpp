// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef PNETE_UI_DOCUMENT_VIEW_HPP
#define PNETE_UI_DOCUMENT_VIEW_HPP

#include <pnete/ui/document_view.fwd.hpp>

#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/data/proxy.fwd.hpp>
#include <pnete/ui/base_editor_widget.fwd.hpp>
#include <pnete/ui/dock_widget.hpp>

#include <boost/optional/optional_fwd.hpp>

#include <QObject>
#include <QSet>

class QString;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class document_view : public dock_widget
      {
        Q_OBJECT;

      public:
        document_view (const data::handle::function&);

        base_editor_widget* widget() const;
        void setWidget (base_editor_widget* widget);

        const QSet<QAction*>& actions() const;

      protected:
        void set_title (const boost::optional<std::string>&);

      public slots:
        void function_name_changed
          (const QObject*, const data::handle::function&, const QString&);

      private slots:
        void visibility_changed (bool);

      private:
        virtual QString fallback_title() const = 0;

        QSet<QAction*> _actions;
      };
    }
  }
}

#endif
