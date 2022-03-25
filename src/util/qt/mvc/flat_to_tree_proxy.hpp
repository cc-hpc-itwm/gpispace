// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
          ~flat_to_tree_proxy() override;
          flat_to_tree_proxy (flat_to_tree_proxy const&) = delete;
          flat_to_tree_proxy (flat_to_tree_proxy&&) = delete;
          flat_to_tree_proxy& operator= (flat_to_tree_proxy const&) = delete;
          flat_to_tree_proxy& operator= (flat_to_tree_proxy&&) = delete;

          virtual int rowCount (QModelIndex const& = QModelIndex()) const override;
          virtual int columnCount (QModelIndex const& = QModelIndex()) const override;
          virtual QModelIndex index
            (int row, int column, QModelIndex const& parent = QModelIndex()) const override;
          virtual QModelIndex parent (QModelIndex const&) const override;
          virtual QVariant data (QModelIndex const&, int role = Qt::DisplayRole) const override;
          virtual QVariant headerData
            (int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
          virtual bool removeRows (int from, int n, QModelIndex const& = QModelIndex()) override;

        private slots:
          void rebuild_transformation_tree();
          void rebuild_source_to_tree();
          void rows_inserted (QModelIndex parent, int from, int to);

          void source_dataChanged (QModelIndex const&, QModelIndex const&);

        private:
          class index_tree_item;

          index_tree_item* item_for (QModelIndex const&) const;
          QModelIndex index_for (index_tree_item*, int column) const;
          QModelIndex index_for (QModelIndex const& source) const;

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

          virtual QVariant data (QModelIndex const&, int role = Qt::DisplayRole) const override;
          virtual bool setData
            (QModelIndex const&, QVariant const&, int role = Qt::EditRole) override;
          virtual QMap<int, QVariant> itemData (QModelIndex const&) const override;
          virtual int rowCount (QModelIndex const& = QModelIndex()) const override;
          virtual bool insertRows (int from, int n, QModelIndex const& = QModelIndex()) override;
          virtual bool removeRows (int from, int n, QModelIndex const& = QModelIndex()) override;

          struct transform_function
          {
            virtual ~transform_function() = default;
            virtual QString operator() (QModelIndex) const = 0;
          };

        private:
          struct item
          {
            item();
            item (QString, ::boost::shared_ptr<transform_function>);

            QString name;
            ::boost::shared_ptr<transform_function> function;
          };

          QList<item> _items;
        };
      }
    }
  }
}

Q_DECLARE_METATYPE
  (::boost::shared_ptr<fhg::util::qt::mvc::transform_functions_model::transform_function>)
