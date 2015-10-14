// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <pnete/plugin/plugin_base.hpp>

namespace demo
{
  class hello_plugin : public fhg::pnete::plugin::plugin_base
  {
    Q_OBJECT

  public:
    hello_plugin (QObject* parent);

    virtual QList<QPair<QString, QList<QAction*>>> menus() const override;
    virtual QList<QPair<QString, QList<QAction*>>> toolbars() const override;

  public slots:
    void greet();
    void set_detailed (bool value);

  private:
    QAction* _hello_action;
    QAction* _detailed_hello_action;
    QWidget _dummy_widget; // _options_menu needs a parent widget
    QMenu* _options_menu;

    bool _detailed;
  };
}
