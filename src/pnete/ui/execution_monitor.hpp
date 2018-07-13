#pragma once

#include <logging/tcp_endpoint.hpp>

#include <QSplitter>

#include <list>

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
                          , std::list<logging::tcp_endpoint>
                          , QWidget* parent = nullptr
                          );
      };
    }
  }
}
