// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/gspc_monitor.hpp>

#include <pnete/ui/node_state_widget.hpp>

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      gspc_monitor::gspc_monitor (QString host, int port, QWidget* parent)
        : QSplitter (Qt::Vertical, parent)
      {
        QWidget* main (new QWidget (this));
        log_widget* log (new log_widget (this));

        QWidget* sidebar (new QWidget (main));
        QScrollArea* content (new QScrollArea (main));

        monitor_client* client (new monitor_client (host, port, main));
        legend* legend_widget (new legend (client, sidebar));

        node_state_widget* node_widget
          (new node_state_widget (host, legend_widget, log, client, content));

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
      }
    }
  }
}
