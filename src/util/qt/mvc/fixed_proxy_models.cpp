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

#include <util/qt/mvc/fixed_proxy_models.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        namespace
        {
          bool fall_back_to_id ( const QAbstractProxyModel* const model
                               , int const& section
                               , Qt::Orientation const& orientation
                               )
          {
            return orientation == Qt::Horizontal
              ? !model->mapToSource (model->index (0, section)).isValid()
              : !model->mapToSource (model->index (section, 0)).isValid();
          }
        }

        abstract_proxy::abstract_proxy (QObject* parent)
          : QAbstractProxyModel (parent)
        {}

        QVariant abstract_proxy::headerData
          (int section, Qt::Orientation orientation, int role) const
        {
          if (fall_back_to_id (this, section, orientation))
          {
            return sourceModel()->headerData (section, orientation, role);
          }
          return QAbstractProxyModel::headerData (section, orientation, role);
        }
        bool abstract_proxy::setHeaderData
          (int section, Qt::Orientation ori, QVariant const& val, int role)
        {
          if (fall_back_to_id (this, section, ori))
          {
            return sourceModel()->setHeaderData (section, ori, val, role);
          }
          return QAbstractProxyModel::setHeaderData (section, ori, val, role);
        }

        QMap<int, QVariant>
          abstract_proxy::itemData (QModelIndex const& index) const
        {
          return sourceModel()->itemData (mapToSource (index));
        }
        bool abstract_proxy::setItemData
          (QModelIndex const& index, QMap<int, QVariant> const& item_data)
        {
          return sourceModel()->setItemData (mapToSource (index), item_data);
        }


        id_proxy::id_proxy (QObject* parent)
          : QIdentityProxyModel (parent)
        {}

        QVariant id_proxy::headerData
          (int section, Qt::Orientation orientation, int role) const
        {
          return sourceModel()->headerData (section, orientation, role);
        }
        bool id_proxy::setHeaderData
          (int section, Qt::Orientation ori, QVariant const& val, int role)
        {
          return sourceModel()->setHeaderData (section, ori, val, role);
        }

        QMap<int, QVariant>
          id_proxy::itemData (QModelIndex const& index) const
        {
          return sourceModel()->itemData (mapToSource (index));
        }
        bool id_proxy::setItemData
          (QModelIndex const& index, QMap<int, QVariant> const& item_data)
        {
          return sourceModel()->setItemData (mapToSource (index), item_data);
        }


        sort_filter_proxy::sort_filter_proxy (QObject* parent)
          : QSortFilterProxyModel (parent)
        {}

        QVariant sort_filter_proxy::headerData
          (int section, Qt::Orientation orientation, int role) const
        {
          if (fall_back_to_id (this, section, orientation))
          {
            return sourceModel()->headerData (section, orientation, role);
          }
          return QSortFilterProxyModel::headerData (section, orientation, role);
        }
        bool sort_filter_proxy::setHeaderData
          (int section, Qt::Orientation ori, QVariant const& val, int role)
        {
          if (fall_back_to_id (this, section, ori))
          {
            return sourceModel()->setHeaderData (section, ori, val, role);
          }
          return QSortFilterProxyModel::setHeaderData (section, ori, val, role);
        }

        QMap<int, QVariant>
          sort_filter_proxy::itemData (QModelIndex const& index) const
        {
          return sourceModel()->itemData (mapToSource (index));
        }
        bool sort_filter_proxy::setItemData
          (QModelIndex const& index, QMap<int, QVariant> const& item_data)
        {
          return sourceModel()->setItemData (mapToSource (index), item_data);
        }
      }
    }
  }
}
