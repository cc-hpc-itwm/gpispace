// Copyright (C) 2013-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QSet>

class QTreeView;



    namespace gspc::util::qt
    {
      class restricted_tree_column_sorter : public QObject
      {
        Q_OBJECT

      public:
        restricted_tree_column_sorter
          (QTreeView*, QSet<int> allowed_columns, QObject* parent = nullptr);

      private:
        enum state
        {
          none,
          asc,
          desc
        };

        void next (int column);

        QTreeView* _tree;
        QSet<int> _allowed_columns;
        state _last_state {none};
        int _last_column {-1};
      };
    }
