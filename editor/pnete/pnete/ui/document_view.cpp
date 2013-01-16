// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/document_view.hpp>

#include <pnete/data/handle/function.hpp>

#include <util/qt/cast.hpp>

#include <xml/parse/type/function.hpp>

#include <boost/optional.hpp>

#include <QStringList>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      document_view::document_view ( const data::handle::function& function
                                   , const data::proxy::type& proxy
                                   , const QString& fallback_title
                                   , QWidget* widget
                                   )
        : dock_widget()
        , _actions()
        , _function (function)
        , _proxy (proxy)
        , _fallback_title (fallback_title)
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

        set_title (function.get().name());

        setWidget (widget);
        widget->setParent (this);
      }

      QList<QAction*> document_view::actions() const
      {
        return widget()->actions();
      }

      data::internal_type* document_view::document() const
      {
        return data::proxy::root (_proxy);
      }
      const data::handle::function& document_view::function() const
      {
        return _function;
      }

      void document_view::function_name_changed
        ( const QObject*
        , const data::handle::function& function
        , const QString& name
        )
      {
        if (function == _function)
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

      void document_view::setWidget (QWidget* widget)
      {
        dock_widget::setWidget (widget);
        widget->setFocusPolicy
          (Qt::FocusPolicy (widget->focusPolicy() | Qt::ClickFocus));
      }

      void document_view::set_title (const boost::optional<std::string>& name)
      {
        setWindowTitle
          (name ? QString::fromStdString (*name) : _fallback_title);
      }
    }
  }
}
