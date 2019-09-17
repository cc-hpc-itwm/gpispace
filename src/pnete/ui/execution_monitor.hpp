#pragma once

#include <logging/endpoint.hpp>

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
                          , std::vector<logging::endpoint>
                          , QWidget* parent = nullptr
                          );
      };
    }
  }
}
