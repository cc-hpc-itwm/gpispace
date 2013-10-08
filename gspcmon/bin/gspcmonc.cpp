#include <pnete/ui/monitor_client.hpp>
#include <pnete/ui/node_state_widget.hpp>

#include <QApplication>
#include <QString>

#include <iostream>
#include <stdexcept>

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

  fhg::pnete::ui::gspc_monitor monitor (host, port);

  monitor.show();

  return app.exec();
}
catch (const std::runtime_error& err)
{
  std::cerr << "error: " << err.what() << "\n";
  return 1;
}
