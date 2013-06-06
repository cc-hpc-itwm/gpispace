#include <gspc/mon/node_state_widget.hpp>

#include <sysexits.h>

#include <QApplication>
#include <QSplitter>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QDir>
#include <QScrollArea>
#include <QGroupBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

int main (int argc, char** argv)
try
{
  QApplication app (argc, argv);

  if (argc != 3)
  {
    std::cerr << "usage: " << argv[0] << " <host> <port>\n";
    return -1;
  }

  const QString host (argv[1]);
  const int port (QString (argv[2]).toInt());

  QSplitter window (Qt::Vertical);

  QWidget* main (new QWidget (&window));
  prefix::log_widget* log (new prefix::log_widget (&window));

  QWidget* sidebar (new QWidget (main));
  prefix::legend* legend_widget (new prefix::legend (sidebar));

  QScrollArea* content (new QScrollArea (main));

  prefix::node_state_widget* node_widget
    (new prefix::node_state_widget (host, port, legend_widget, log, content));

  content->setWidget (node_widget);
  content->setWidgetResizable (true);

  {
    QGroupBox* sort_box (new QGroupBox (QObject::tr ("sort"), sidebar));

    {
      QPushButton* sort_by_state
        (new QPushButton (QObject::tr ("by state"), sort_box));
      QPushButton* sort_by_name
        (new QPushButton (QObject::tr ("by name"), sort_box));

      node_widget->connect
        (sort_by_state, SIGNAL (clicked()), SLOT (sort_by_state()));
      node_widget->connect
        (sort_by_name, SIGNAL (clicked()), SLOT (sort_by_name()));

      QVBoxLayout* layout (new QVBoxLayout (sort_box));
      layout->addWidget (sort_by_state);
      layout->addWidget (sort_by_name);
    }

    QPushButton* clear_log
      (new QPushButton (QObject::tr ("clear log"), sidebar));
    log->connect (clear_log, SIGNAL (clicked()), SLOT (clear()));

    QCheckBox* follow_logging
      (new QCheckBox (QObject::tr ("follow logging"), sidebar));
    log->connect (follow_logging, SIGNAL (toggled (bool)), SLOT (follow (bool)));
    follow_logging->setChecked (true);

    QVBoxLayout* layout (new QVBoxLayout (sidebar));
    layout->addWidget (legend_widget);
    layout->addWidget (sort_box);
    layout->addStretch();
    layout->addWidget (follow_logging);
    layout->addWidget (clear_log);
  }

  {
    QHBoxLayout* layout (new QHBoxLayout (main));
    layout->addWidget (content);
    layout->addWidget (sidebar);
  }

  window.show();

  return app.exec();
}
catch (const prefix::connection_error& err)
{
  std::cerr << "failed to connect: " << err.what() << "\n";
  return 1;
}
catch (const std::runtime_error& err)
{
  std::cerr << "error: " << err.what() << "\n";
  return 1;
}
