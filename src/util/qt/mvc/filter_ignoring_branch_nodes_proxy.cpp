// bernd.loerwald@itwm.fraunhofer.de

#include <util/qt/mvc/filter_ignoring_branch_nodes_proxy.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        filter_ignoring_branch_nodes_proxy::filter_ignoring_branch_nodes_proxy
          (QAbstractItemModel* model, QObject* parent)
            : sort_filter_proxy (parent)
        {
          setSourceModel (model);
        }

        bool filter_ignoring_branch_nodes_proxy::filterAcceptsRow
          (int source_row, const QModelIndex& parent) const
        {
          if (sort_filter_proxy::filterAcceptsRow (source_row, parent))
          {
            return true;
          }

          const QModelIndex idx
            (sourceModel()->index (source_row, filterKeyColumn(), parent));

          // p is visible if \exists child^n (p) \in visible.

          for (int row (0); row < sourceModel()->rowCount (idx); ++row)
          {
            if (filterAcceptsRow (row, idx))
            {
              return true;
            }
          }

          // ! \todo Is l visible, if \exists parent^n (l) \in visible?

          return false;
        }
      }
    }
  }
}
