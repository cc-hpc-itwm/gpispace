#pragma once

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

#include <QSplitter>

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
        execution_monitor (unsigned short port,
            boost::optional<boost::filesystem::path> const trace_file,
            QWidget* parent = nullptr);
      };
    }
  }
}
