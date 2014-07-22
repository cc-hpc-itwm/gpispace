// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/plugin/plugin_api.hpp>

#include <pnete/plugin/demo/hello_world.hpp>

#include <QMessageBox>
#include <QMenu>

namespace demo
{
  hello_plugin::hello_plugin (QObject* parent)
    : fhg::pnete::plugin::plugin_base (parent)
    , _hello_action (new QAction (tr ("hello_foo"), this))
    , _detailed_hello_action (new QAction (tr ("hello_detailed_foo"), this))
    , _dummy_widget()
    , _options_menu (new QMenu (tr ("options"), &_dummy_widget))
    , _detailed (true)
  {
    connect (_hello_action, SIGNAL (triggered (bool)), this, SLOT (greet()));

    _detailed_hello_action->setCheckable (true);
    _detailed_hello_action->setChecked (_detailed);
    connect ( _detailed_hello_action, SIGNAL (triggered (bool))
            , this, SLOT (set_detailed (bool))
            );

    _options_menu->addAction (_detailed_hello_action);
  }

  QList<QPair<QString, QList<QAction*>>> hello_plugin::menus() const
  {
    QList<QAction*> actions;
    actions << _hello_action;
    actions << _options_menu->menuAction();

    QList<QPair<QString, QList<QAction*>>> menus;
    menus << qMakePair (QString ("greeting_menu"), actions);
    return menus;
  }
  QList<QPair<QString, QList<QAction*>>> hello_plugin::toolbars() const
  {
    QList<QAction*> actions;
    actions << _hello_action;
    actions << _detailed_hello_action;

    QList<QPair<QString, QList<QAction*>>> toolbars;
    toolbars << qMakePair (QString ("greeting_bar"), actions);
    return toolbars;
  }

  void hello_plugin::greet()
  {
    if (_detailed)
    {
      QMessageBox::information (nullptr, "greeting (detailed)", "hello mr. foo");
    }
    else
    {
      QMessageBox::information (nullptr, "greeting", "hello foo");
    }
  }
  void hello_plugin::set_detailed (bool value)
  {
    _detailed = value;
  }
}

fhg::pnete::plugin::plugin_base* fhg_pnete_create_plugin (QObject* parent)
{
  return new demo::hello_plugin (parent);
}
