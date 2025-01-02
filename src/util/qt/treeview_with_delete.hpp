// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTreeView>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class treeview_with_delete : public QTreeView
      {
        Q_OBJECT

      public:
        treeview_with_delete (QWidget* parent = nullptr);

      protected:
        void keyPressEvent (QKeyEvent*) override;
      };
    }
  }
}
