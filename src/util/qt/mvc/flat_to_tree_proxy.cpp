// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util/qt/mvc/flat_to_tree_proxy.hpp>

#include <util-qt/variant.hpp>

#include <fhg/assert.hpp>

#include <QSet>
#include <QStringList>
#include <QVector>

#include <variant>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        transform_functions_model::transform_functions_model (QObject* parent)
          : QAbstractListModel (parent)
        { }

        QVariant transform_functions_model::data
          (QModelIndex const& index, int role) const
        {
          if (index.isValid() && index.row() < _items.size())
          {
            if (role == function_role)
            {
              return QVariant::fromValue (_items[index.row()].function);
            }
            else if (role == Qt::DisplayRole)
            {
              return _items[index.row()].name;
            }
          }

          return QVariant();
        }
        bool transform_functions_model::setData
          (QModelIndex const& index, QVariant const& variant, int role)
        {
          if (!index.isValid() || index.row() >= _items.size())
          {
            return false;
          }

          if (role == function_role)
          {
            _items[index.row()].function
              = value<::boost::shared_ptr<transform_function>> (variant);
          }
          else if (role == Qt::DisplayRole)
          {
            _items[index.row()].name = value<QString> (variant);
          }
          else
          {
            return false;
          }

          emit dataChanged (index, index);
          return true;
        }

        QMap<int, QVariant>
          transform_functions_model::itemData (QModelIndex const& index) const
        {
          QMap<int, QVariant> roles (QAbstractListModel::itemData (index));
          roles[function_role] = data (index, function_role);
          return roles;
        }

        int transform_functions_model::rowCount (QModelIndex const& index) const
        {
          return index.isValid() ? 0 : _items.size();
        }

        namespace
        {
          struct transform_nop
            : public util::qt::mvc::transform_functions_model::transform_function
          {
            QString operator() (QModelIndex index) const override
            {
              return index.data().toString();
            }
          };
        }

        transform_functions_model::item::item
          (QString name_, ::boost::shared_ptr<transform_function> function_)
            : name (name_)
            , function (function_)
        { }
        transform_functions_model::item::item()
          : name()
          , function (new transform_nop)
        { }

        bool transform_functions_model::insertRows
          (int row, int count, QModelIndex const& parent)
        {
          fhg_assert (!parent.isValid(), "there are no rows on non-top-level");

          beginInsertRows (parent, row, row + count - 1);
          while (count --> 0)
          {
            _items.insert (row, item());
          }
          endInsertRows();
          return true;
        }

        bool transform_functions_model::removeRows
          (int row, int count, QModelIndex const& parent)
        {
          fhg_assert (!parent.isValid(), "there are no rows on non-top-level");

          beginRemoveRows (parent, row, row + count - 1);
          while (count --> 0)
          {
            _items.removeAt (row);
          }
          endRemoveRows();
          return true;
        }

        class flat_to_tree_proxy::index_tree_item
        {
        public:
          //! \note Branch
          index_tree_item (QString const& name, index_tree_item* parent)
            : _parent (parent)
            , _data (std::make_pair (name, QVector<index_tree_item*>()))
          {
            register_in_parent();
          }

          //! \note Leaf
          index_tree_item (QPersistentModelIndex const& index, index_tree_item* parent)
            : _parent (parent)
            , _data (index)
          {
            register_in_parent();
          }

          ~index_tree_item()
          {
            if (is_branch())
            {
              for (index_tree_item* child : children())
              {
                delete child;
              }
            }
          }

          index_tree_item (index_tree_item const&) = delete;
          index_tree_item& operator= (index_tree_item const&) = delete;
          index_tree_item (index_tree_item&&) = delete;
          index_tree_item& operator= (index_tree_item&&) = delete;

          bool is_leaf() const
          {
            return std::holds_alternative<index_type> (_data);
          }
          bool is_branch() const
          {
            return std::holds_alternative<name_and_child_type> (_data);
          }

          const QVector<index_tree_item*>& children() const
          {
            fhg_assert (is_branch(), "children() only to be called on branch");
            return std::get<name_and_child_type> (_data).second;
          }
          QString const& name() const
          {
            fhg_assert (is_branch(), "name() only to be called on branch");
            return std::get<name_and_child_type> (_data).first;
          }
          QPersistentModelIndex const& index() const
          {
            fhg_assert (is_leaf(), "index() only to be called on leaf");
            return std::get<index_type> (_data);
          }

          index_tree_item* parent() const
          {
            return _parent;
          }

          QSet<index_tree_item*> all_leafs_below()
          {
            QSet<index_tree_item*> result;
            all_leafs_below (result);
            return result;
          }

          index_tree_item* find_branch (QString name)
          {
            for (index_tree_item* item : children())
            {
              if (item->is_branch() && item->name() == name)
              {
                return item;
              }
            }
            return nullptr;
          }

          void remove_child (int index)
          {
            fhg_assert (is_branch(), "remove_child() only to be called on branch");

            std::get<name_and_child_type> (_data).second.remove (index);
          }

        private:
          void register_in_parent()
          {
            if (_parent)
            {
              fhg_assert (_parent->is_branch(), "parent needs to be a branch");
              std::get<name_and_child_type> (_parent->_data).second << this;
            }
          }

          void all_leafs_below (QSet<index_tree_item*>& above)
          {
            if (is_leaf())
            {
              above << this;
            }
            else
            {
              for (index_tree_item* child : children())
              {
                child->all_leafs_below (above);
              }
            }
          }

          index_tree_item* _parent;

          using name_and_child_type =
            std::pair<QString, QVector<index_tree_item*>>;
          using index_type = QPersistentModelIndex;

          std::variant<name_and_child_type, index_type> _data;
        };

        flat_to_tree_proxy::flat_to_tree_proxy
          ( QAbstractItemModel* source
          , transform_functions_model* transforms
          , QObject* parent
          )
            : QAbstractItemModel (parent)
            , _source (source)
            , _transform_functions (transforms)
            , _invisible_root (nullptr)
        {
          connect ( source
                  , SIGNAL (columnsAboutToBeInserted (QModelIndex, int, int))
                  , SIGNAL (columnsAboutToBeInserted (QModelIndex, int, int))
                  );
          connect ( source
                  , SIGNAL (columnsAboutToBeMoved (QModelIndex, int, int, QModelIndex, int))
                  , SIGNAL (columnsAboutToBeMoved (QModelIndex, int, int, QModelIndex, int))
                  );
          connect ( source
                  , SIGNAL (columnsAboutToBeRemoved (QModelIndex, int, int))
                  , SIGNAL (columnsAboutToBeRemoved (QModelIndex, int, int))
                  );
          connect ( source
                  , SIGNAL (columnsInserted (QModelIndex, int, int))
                  , SIGNAL (columnsInserted (QModelIndex, int, int))
                  );
          connect ( source
                  , SIGNAL (columnsMoved (QModelIndex, int, int, QModelIndex, int))
                  , SIGNAL (columnsMoved (QModelIndex, int, int, QModelIndex, int))
                  );
          connect ( source
                  , SIGNAL (columnsRemoved (QModelIndex, int, int))
                  , SIGNAL (columnsRemoved (QModelIndex, int, int))
                  );
          connect ( source
                  , SIGNAL (headerDataChanged (Qt::Orientation, int, int))
                  , SIGNAL (headerDataChanged (Qt::Orientation, int, int))
                  );

          connect ( source, SIGNAL (dataChanged (QModelIndex, QModelIndex))
                  , SLOT (source_dataChanged (QModelIndex, QModelIndex))
                  );

          // layoutAboutToBeChanged() -> layoutChanged()
          connect ( source, SIGNAL (layoutChanged())
                  , SLOT (rebuild_transformation_tree())
                  );
          // modelAboutToBeReset() -> modelReset()
          connect ( source, SIGNAL (modelReset())
                  , SLOT (rebuild_transformation_tree())
                  );

          // rowsAboutToBeInserted -> rowsInserted
          // rowsAboutToBeMoved -> rowsMoved
          // rowsAboutToBeRemoved -> rowsRemoved
          connect ( source
                  , SIGNAL (rowsInserted (QModelIndex, int, int))
                  , SLOT (rows_inserted (QModelIndex, int, int))
                  );
          connect ( source
                  , SIGNAL (rowsMoved (QModelIndex, int, int, QModelIndex, int))
                  , SLOT (rebuild_transformation_tree())
                  );
          connect ( source
                  , SIGNAL (rowsRemoved (QModelIndex, int, int))
                  , SLOT (rebuild_source_to_tree())
                  );


          connect ( _transform_functions
                  , SIGNAL (dataChanged (QModelIndex, QModelIndex))
                  , SLOT (rebuild_transformation_tree())
                  );
          connect ( _transform_functions
                  , SIGNAL (layoutChanged())
                  , SLOT (rebuild_transformation_tree())
                  );
          connect ( _transform_functions
                  , SIGNAL (modelReset())
                  , SLOT (rebuild_transformation_tree())
                  );
          connect ( _transform_functions
                  , SIGNAL (rowsInserted (QModelIndex, int, int))
                  , SLOT (rebuild_transformation_tree())
                  );
          connect ( _transform_functions
                  , SIGNAL (rowsMoved (QModelIndex, int, int, QModelIndex, int))
                  , SLOT (rebuild_transformation_tree())
                  );
          connect ( _transform_functions
                  , SIGNAL (rowsRemoved (QModelIndex, int, int))
                  , SLOT (rebuild_transformation_tree())
                  );


          rebuild_transformation_tree();
        }

        flat_to_tree_proxy::~flat_to_tree_proxy()
        {
        }

        void flat_to_tree_proxy::source_dataChanged
          (QModelIndex const& topLeft, QModelIndex const& bottomRight)
        {
#ifdef collect_and_combine
          QMap<QModelIndex, std::set<int>> changed_child_rows;

          for (int row (topLeft.row()); row <= bottomRight.row(); ++row)
          {
            for (int col (topLeft.column()); col <= bottomRight.column(); ++col)
            {
              QModelIndex index (index_for (topLeft.sibling (row, col)));
              while (index.isValid())
              {
                const QModelIndex parent (index.parent());
                changed_child_rows[parent].insert (index.row());
                index = parent;
              }
            }
          }

          for (QModelIndex const& parent : changed_child_rows.keys())
          {
            std::set<int> const& rows (changed_child_rows[parent]);
            std::set<int>::iterator it (rows.begin());

            int last (*it);
            int first (last);

            for (++it; it != rows.end(); ++it)
            {
              if (*it != last + 1)
              {
                emit dataChanged ( index (first, topLeft.column(), parent)
                                 , index (last, bottomRight.column(), parent)
                                 );
                first = *it;
              }
              last = *it;
            }
            emit dataChanged ( index (first, topLeft.column(), parent)
                             , index (last, bottomRight.column(), parent)
                             );
          }
#else
          if (topLeft == bottomRight)
          {
            QModelIndex index (index_for (topLeft));
            while (index.isValid())
            {
              emit dataChanged (index, index);
              index = index.parent();
            }
          }
          else
          {
#ifdef individually_issue_changes_for_all_inbetween
            for (int row (topLeft.row()); row <= bottomRight.row(); ++row)
            {
              for (int col (topLeft.column()); col <= bottomRight.column(); ++col)
              {
                const QModelIndex index (topLeft.sibling (row, col));
                source_dataChanged (index, index);
              }
            }
#else
            emit dataChanged ( index (0, topLeft.column(), QModelIndex())
                             , index (rowCount() - 1, bottomRight.column(), QModelIndex())
                             );
#endif
          }
#endif
        }

        namespace
        {
          template<typename T>
            QList<T> value_of_all_rows ( const QAbstractItemModel* const model
                                       , int role
                                       , int column
                                       )
          {
            QList<T> ret;
            const int rows (model->rowCount());
            for (int i (0); i < rows; ++i)
            {
              ret << value<T> (model->index (i, column).data (role));
            }
            return ret;
          }
        }

        void flat_to_tree_proxy::rebuild_transformation_tree()
        {
          beginResetModel();

          _invisible_root.reset
            ( new index_tree_item
              ("<<invisible root, you should never see this>>", nullptr)
            );

          insert_from_source (0, _source->rowCount(), QModelIndex(), false);

          endResetModel();
        }

        void flat_to_tree_proxy::rows_inserted
          (QModelIndex parent, int from, int to)
        {
          insert_from_source (from, to + 1, parent, true);
        }

        void flat_to_tree_proxy::insert_from_source
          (int begin, int end, QModelIndex parent, bool emit_per_row)
        {
          const QList<::boost::shared_ptr<transform_functions_model::transform_function>> transform_functions
            ( value_of_all_rows<::boost::shared_ptr<transform_functions_model::transform_function>>
              ( _transform_functions
              , transform_functions_model::function_role
              , 0
              )
            );

          for (int row (begin); row < end; ++row)
          {
            const QModelIndex index (_source->index (row, 0, parent));

            QStringList path;
            for ( ::boost::shared_ptr<transform_functions_model::transform_function> fun
                : transform_functions
                )
            {
              path.prepend ((*fun) (index));
            }

            index_tree_item* last_item (_invisible_root.get());
            QModelIndex last_parent;
            for (QString const& segment : path)
            {
              index_tree_item* branch (last_item->find_branch (segment));
              if (!branch)
              {
                if (emit_per_row)
                {
                  const int rows (rowCount (last_parent));
                  beginInsertRows (last_parent, rows, rows);
                }
                last_item = new index_tree_item (segment, last_item);
                if (emit_per_row)
                {
                  endInsertRows();
                }
              }
              else
              {
                last_item = branch;
              }
              last_parent = index_for (last_item, 0);
            }

            const int leaf_parent_rows (rowCount (last_parent));
            if (emit_per_row)
            {
              beginInsertRows (last_parent, leaf_parent_rows, leaf_parent_rows);
            }

            auto* leaf (new index_tree_item (index, last_item));

            if (emit_per_row)
            {
              endInsertRows();
            }

            for (int col (0), col_count (_source->columnCount()); col < col_count; ++col)
            {
              _source_to_tree[_source->index (row, col, QModelIndex())]
                = index_for (leaf, col);
            }
          }
        }

        void flat_to_tree_proxy::rebuild_source_to_tree()
        {
          _source_to_tree.clear();

          for (index_tree_item* item : _invisible_root->all_leafs_below())
          {
            if (!item->index().isValid())
            {
              int row (item->parent()->children().indexOf (item));
              beginRemoveRows (index_for (item->parent(), 0), row, row);
              item->parent()->remove_child (row);
              endRemoveRows();
            }
            else
            {
              for (int col (0), col_count (_source->columnCount()); col < col_count; ++col)
              {
                _source_to_tree[item->index().sibling (item->index().row(), col)]
                  = index_for (item, col);
              }
            }
          }
        }

        flat_to_tree_proxy::index_tree_item*
          flat_to_tree_proxy::item_for (QModelIndex const& index) const
        {
          return index.isValid()
            ? static_cast<index_tree_item*> (index.internalPointer())
            : _invisible_root.get();
        }

        int flat_to_tree_proxy::rowCount (QModelIndex const& index) const
        {
          index_tree_item* item (item_for (index));
          return item->is_leaf() ? 0 : item->children().size();
        }
        int flat_to_tree_proxy::columnCount (QModelIndex const& index) const
        {
          index_tree_item* item (item_for (index));
          return _source->columnCount ( item->is_leaf()
                                      ? QModelIndex (item->index())
                                      : QModelIndex()
                                      );
        }

        QModelIndex flat_to_tree_proxy::index_for
          (index_tree_item* item, int column) const
        {
          if (item && item->parent())
          {
            int row (0);
            for (index_tree_item* child : item->parent()->children())
            {
              if (child == item)
              {
                return createIndex (row, column, item);
              }
              ++row;
            }
          }
          return QModelIndex();
        }

        QModelIndex flat_to_tree_proxy::index_for (QModelIndex const& source) const
        {
          return _source_to_tree[source];
        }

        QModelIndex flat_to_tree_proxy::index
          (int row, int column, QModelIndex const& parent) const
        {
          return createIndex (row, column, item_for (parent)->children()[row]);
        }
        QModelIndex flat_to_tree_proxy::parent (QModelIndex const& index) const
        {
          return index_for
            ( static_cast<index_tree_item*> (index.internalPointer())->parent()
            , index.column()
            );
        }

        QVariant flat_to_tree_proxy::data (QModelIndex const& index, int role) const
        {
          index_tree_item* item (item_for (index));
          if (item->is_leaf())
          {
            return item->index().data (role);
          }

          switch (role)
          {
          case Qt::DisplayRole:
            return item->name();

            //! \note Could as well check for role < Qt::UserRole, I guess.
          case Qt::AccessibleDescriptionRole:
          case Qt::AccessibleTextRole:
          case Qt::BackgroundRole:
          case Qt::CheckStateRole:
          case Qt::DecorationRole:
          case Qt::EditRole:
          case Qt::FontRole:
          case Qt::ForegroundRole:
          case Qt::InitialSortOrderRole:
          case Qt::SizeHintRole:
          case Qt::StatusTipRole:
          case Qt::TextAlignmentRole:
          case Qt::ToolTipRole:
          case Qt::WhatsThisRole:
            return QVariant();

          default:
            QVariantList data;
            for (const index_tree_item* leaf : item->all_leafs_below())
            {
              data << leaf->index().data (role);
            }
            return QVariant::fromValue (data);
          }
        }

        QVariant flat_to_tree_proxy::headerData
          (int section, Qt::Orientation orientation, int role) const
        {
          return _source->headerData (section, orientation, role);
        }

        bool flat_to_tree_proxy::removeRows
          (int row, int count, QModelIndex const& parent)
        {
          for (const int max (row + count); row < max; ++row)
          {
            QSet<QPersistentModelIndex> indices;

            for ( const index_tree_item* const item
                : item_for (parent)->children()[row]->all_leafs_below()
                )
            {
              indices.insert (item->index());
            }
            for (QPersistentModelIndex index : indices)
            {
              _source->removeRow (index.row(), index.parent());
            }
          }

          return true;
        }
      }
    }
  }
}
