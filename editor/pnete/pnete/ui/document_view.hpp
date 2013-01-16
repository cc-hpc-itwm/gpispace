// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef PNETE_UI_DOCUMENT_VIEW_HPP
#define PNETE_UI_DOCUMENT_VIEW_HPP

#include <pnete/ui/document_view.fwd.hpp>

#include <pnete/data/handle/function.hpp>
#include <pnete/data/proxy.hpp>
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
        document_view ( const data::handle::function&
                      , const data::proxy::type&
                      , const QString& fallback_title
                      , QWidget*
                      );

        void setWidget (QWidget* widget);

        QList<QAction*> actions() const;

        const data::proxy::type& proxy() const;
        const data::handle::function& function() const;

      protected:
        void set_title (const boost::optional<std::string>&);

      public slots:
        void function_name_changed
          (const QObject*, const data::handle::function&, const QString&);

      private slots:
        void visibility_changed (bool);

      private:
        QSet<QAction*> _actions;
        data::handle::function _function;
        data::proxy::type _proxy;
        QString _fallback_title;
      };
    }
  }
}

#endif
