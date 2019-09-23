#pragma once

#include <logging/message.hpp>

#include <QSplitter>

#include <vector>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class worker_model;

      class execution_monitor : public QSplitter
      {
        Q_OBJECT

      public:
        execution_monitor();

        void append_event (logging::message const&);

      private:
        worker_model* base;
      };
    }
  }
}
