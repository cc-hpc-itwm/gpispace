#pragma once

#include <logging/tcp_endpoint.hpp>

#include <QSplitter>

#include <vector>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class execution_monitor : public QSplitter
      {
        Q_OBJECT

      public:
        execution_monitor ( unsigned short port
                          , std::vector<logging::tcp_endpoint>
                          , QWidget* parent = nullptr
                          );
      };
    }
  }
}
