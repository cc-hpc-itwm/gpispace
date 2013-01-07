// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/document_view.hpp>

#include <pnete/data/handle/function.hpp>
#include <pnete/ui/base_editor_widget.hpp>

#include <util/qt/cast.hpp>

#include <boost/optional.hpp>

#include <QStringList>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      document_view::document_view (const data::handle::function& function)
        : dock_widget()
        , _actions()
      {
        connect ( this
                , SIGNAL (visibilityChanged (bool))
                , SLOT (visibility_changed (bool))
                );

        function.connect_to_change_mgr
          ( this
          , "function_name_changed"
          , "data::handle::function, QString"
          );
      }

      const QSet<QAction*>& document_view::actions() const
      {
        return _actions;
      }

      void document_view::function_name_changed
        ( const QObject*
        , const data::handle::function& function
        , const QString& name
        )
      {
        if (function == widget()->function())
        {
          set_title
            (boost::make_optional (!name.isEmpty(), name.toStdString()));
        }
      }

      void document_view::visibility_changed (bool visible)
      {
        if (visible)
        {
          setFocus();
        }
      }

      base_editor_widget* document_view::widget() const
      {
        return util::qt::throwing_qobject_cast<base_editor_widget*>
          (dock_widget::widget());
      }
      void document_view::setWidget (base_editor_widget* widget)
      {
        dock_widget::setWidget (widget);
        widget->setFocusPolicy
          (Qt::FocusPolicy (widget->focusPolicy() | Qt::ClickFocus));
      }

      void document_view::set_title (const boost::optional<std::string>& name)
      {
        setWindowTitle
          (name ? QString::fromStdString (*name) : fallback_title());
      }
    }
  }
}
