// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_RESTRICTED_TREE_COLUMN_SORTER_HPP
#define FHG_UTIL_QT_RESTRICTED_TREE_COLUMN_SORTER_HPP

#include <QSet>

class QTreeView;

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class restricted_tree_column_sorter
      {
      public:
        restricted_tree_column_sorter
          (QTreeView*, const QSet<int> allowed_columns);

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

#endif
