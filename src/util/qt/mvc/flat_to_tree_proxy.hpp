// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <QAbstractItemModel>
#include <QList>
#include <QMap>
#include <QVariant>

#include <boost/shared_ptr.hpp>

#include <memory>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        class transform_functions_model;

        class flat_to_tree_proxy : public QAbstractItemModel
        {
          Q_OBJECT

        public:
          flat_to_tree_proxy ( QAbstractItemModel* source
                             , transform_functions_model*
                             , QObject* parent = nullptr
                             );
          //! \note Exists for ~unique_ptr<fwd-decl-type>() only.
          ~flat_to_tree_proxy();

          virtual int rowCount (const QModelIndex& = QModelIndex()) const override;
          virtual int columnCount (const QModelIndex& = QModelIndex()) const override;
          virtual QModelIndex index
            (int row, int column, const QModelIndex& parent = QModelIndex()) const override;
          virtual QModelIndex parent (const QModelIndex&) const override;
          virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const override;
          virtual QVariant headerData
            (int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
          virtual bool removeRows (int from, int n, const QModelIndex& = QModelIndex()) override;

        private slots:
          void rebuild_transformation_tree();
          void rebuild_source_to_tree();
          void rows_inserted (QModelIndex parent, int from, int to);

          void source_dataChanged (const QModelIndex&, const QModelIndex&);

        private:
          class index_tree_item;

          index_tree_item* item_for (const QModelIndex&) const;
          QModelIndex index_for (index_tree_item*, int column) const;
          QModelIndex index_for (const QModelIndex& source) const;

          void insert_from_source
            (int begin, int end, QModelIndex parent, bool emit_per_row);

          QAbstractItemModel* _source;

          transform_functions_model* _transform_functions;
          std::unique_ptr<index_tree_item> _invisible_root;
          QMap<QModelIndex, QModelIndex> _source_to_tree;
        };

        class transform_functions_model : public QAbstractListModel
        {
          Q_OBJECT

        public:
          enum
          {
            function_role = Qt::UserRole
          };

          transform_functions_model (QObject* parent = nullptr);

          virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const override;
          virtual bool setData
            (const QModelIndex&, const QVariant&, int role = Qt::EditRole) override;
          virtual QMap<int, QVariant> itemData (const QModelIndex&) const override;
          virtual int rowCount (const QModelIndex& = QModelIndex()) const override;
          virtual bool insertRows (int from, int n, const QModelIndex& = QModelIndex()) override;
          virtual bool removeRows (int from, int n, const QModelIndex& = QModelIndex()) override;

          struct transform_function
          {
            virtual ~transform_function() = default;
            virtual QString operator() (QModelIndex) const = 0;
          };

        private:
          struct item
          {
            item();
            item (QString, boost::shared_ptr<transform_function>);

            QString name;
            boost::shared_ptr<transform_function> function;
          };

          QList<item> _items;
        };
      }
    }
  }
}

Q_DECLARE_METATYPE
  (boost::shared_ptr<fhg::util::qt::mvc::transform_functions_model::transform_function>)
