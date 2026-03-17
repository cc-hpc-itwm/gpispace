// Copyright (C) 2013-2015,2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTreeView>



    namespace gspc::util::qt
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
