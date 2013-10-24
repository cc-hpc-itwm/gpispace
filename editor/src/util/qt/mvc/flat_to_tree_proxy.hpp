// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_MVC_FLAT_TO_TREE_PROXY_HPP
#define FHG_UTIL_QT_MVC_FLAT_TO_TREE_PROXY_HPP

#include <QAbstractItemModel>
#include <QList>
#include <QMap>
#include <QVariant>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

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
                             , QObject* parent = NULL
                             );
          //! \note Exists for ~scoped_ptr<fwd-decl-type>() only.
          ~flat_to_tree_proxy();

          virtual int rowCount (const QModelIndex& = QModelIndex()) const;
          virtual int columnCount (const QModelIndex& = QModelIndex()) const;
          virtual QModelIndex index
            (int row, int column, const QModelIndex& parent = QModelIndex()) const;
          virtual QModelIndex parent (const QModelIndex&) const;
          virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
          virtual QVariant headerData
            (int section, Qt::Orientation, int role = Qt::DisplayRole) const;
          virtual bool removeRows (int from, int n, const QModelIndex& = QModelIndex());

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
          boost::scoped_ptr<index_tree_item> _invisible_root;
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

          transform_functions_model (QObject* parent = NULL);

          virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
          virtual bool setData
            (const QModelIndex&, const QVariant&, int role = Qt::EditRole);
          virtual QMap<int, QVariant> itemData (const QModelIndex&) const;
          virtual int rowCount (const QModelIndex& = QModelIndex()) const;
          virtual bool insertRows (int from, int n, const QModelIndex& = QModelIndex());
          virtual bool removeRows (int from, int n, const QModelIndex& = QModelIndex());

          struct transform_function
          {
            virtual ~transform_function() { }
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

#endif
