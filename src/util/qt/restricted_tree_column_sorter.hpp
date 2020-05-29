#pragma once

#include <QObject>
#include <QSet>

class QTreeView;

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class restricted_tree_column_sorter : public QObject
      {
        Q_OBJECT

      public:
        restricted_tree_column_sorter
          (QTreeView*, const QSet<int> allowed_columns, QObject* parent = nullptr);

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
        state _last_state;
        int _last_column;
      };
    }
  }
}
