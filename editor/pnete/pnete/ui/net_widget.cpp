// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/net_widget.hpp>

#include <pnete/data/handle/function.hpp>
#include <pnete/ui/graph_view.hpp>
#include <pnete/ui/port_lists_widget.hpp>
#include <pnete/weaver/display.hpp>

#include <xml/parse/type/function.hpp>

#include <util/qt/scoped_signal_block.hpp>

#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      net_widget::net_widget ( const data::handle::net& net
                             , const data::handle::function& function
                             , QWidget* parent
                             )
        : QSplitter (parent)
        , _function (function)
        , _name_edit (new QLineEdit())
      {
        QWidget* exp_widget (new QWidget (this));
        QVBoxLayout* vbox (new QVBoxLayout (exp_widget));

        QWidget* name_widget (new QWidget (exp_widget));
        (new QFormLayout (name_widget))->addRow (tr ("&Name"), _name_edit);

        vbox->addWidget (name_widget);
        vbox->addWidget
          (new port_lists_widget (_function, QStringList(), exp_widget));

        const graph_view* const gv
          (new graph_view (weaver::display::net (net, function), this));
        addActions (gv->actions());

        _function.connect_to_change_mgr
          ( this
          , "function_name_changed", "name_changed"
          , "data::handle::function, QString"
          );

        set_name
          (QString::fromStdString (_function.get().name().get_value_or ("")));

        connect ( _name_edit
                , SIGNAL (textEdited (QString))
                , SLOT (name_edit_changed (QString))
                );
      }

      void net_widget::set_name (const QString& name)
      {
        const util::qt::scoped_signal_block block (_name_edit);
        _name_edit->setText (name);
      }

      void net_widget::name_edit_changed (const QString& name)
      {
        _function.set_name (this, name);
      }

      void net_widget::name_changed
        ( const QObject*
        , const data::handle::function& fun
        , const QString& name
        )
      {
        if (is_my_function (fun))
        {
          set_name (name);
        }
      }

      bool net_widget::is_my_function
        (const data::handle::function& f)
      {
        return f == _function;
      }
    }
  }
}
