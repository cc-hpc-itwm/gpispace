// bernd.loerwald@itwm.fraunhofer.de

#include <util/qt/mvc/multiplicated_column_proxy.hpp>

#include <util/qt/scoped_disconnect.hpp>

#include <fhg/assert.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        multiplicated_column_proxy::multiplicated_column_proxy
          (QAbstractItemModel* model, QObject* parent)
            : id_proxy (parent)
            , _column_count (0)
        {
          setSourceModel (model);
          connect ( this, SIGNAL (dataChanged (QModelIndex,QModelIndex))
                  , SLOT (source_dataChanged (QModelIndex,QModelIndex))
                  );
        }

        void multiplicated_column_proxy::source_dataChanged
          (const QModelIndex& from, const QModelIndex& to)
        {
          fhg_assert
            (from.column() == 0, "source model shall only have one column");

          const scoped_disconnect disconnecter
            ( this, SIGNAL (dataChanged (QModelIndex,QModelIndex))
            , this, SLOT (source_dataChanged (QModelIndex,QModelIndex))
            );

          for (int row (from.row()); row <= to.row(); ++row)
          {
            for (int col (1); col < _column_count; ++col)
            {
              const QModelIndex index
                (createIndex (row, col, from.sibling (row, 0).internalPointer()));
              emit dataChanged (index, index);
            }
          }
        }

        int multiplicated_column_proxy::columnCount (const QModelIndex&) const
        {
          return _column_count;
        }

        bool multiplicated_column_proxy::insertColumns
          (int column, int count, const QModelIndex& parent)
        {
          if (parent.isValid())
          {
            return id_proxy::insertColumns (column, count, parent);
          }

          beginInsertColumns (parent, column, column + count - 1);
          _column_count += count;
          endInsertColumns();
          return true;
        }

        bool multiplicated_column_proxy::removeColumns
          (int column, int count, const QModelIndex& parent)
        {
          if (parent.isValid())
          {
            return id_proxy::removeColumns (column, count, parent);
          }

          beginRemoveColumns (parent, column, column + count - 1);
          _column_count -= count;
          endRemoveColumns();
          return true;
        }

        QModelIndex multiplicated_column_proxy::mapToSource
          (const QModelIndex& proxy) const
        {
          return id_proxy::mapToSource
            (createIndex (proxy.row(), 0, proxy.internalPointer()));
        }
      }
    }
  }
}
