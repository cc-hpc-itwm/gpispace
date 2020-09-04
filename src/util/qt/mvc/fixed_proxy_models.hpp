// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

//! \note Qt provides an API for mapping indices, thus changing
//! columns in proxy models. There is no way at all, to do the same
//! for whole columns though, which is why Qt itself relies on mapping
//! cell [0, n] when looking for mapping of column n. For lookup,
//! mapToSource (index (0, n)) is used, where index (0, n) will result
//! in an invalid index though, when used on an empty source
//! model. The resulting column thus is -1, as no check for
//! mapped-index-validity is in place. This will break headerData()
//! and quite some others. This file fixes the problem for
//! headerData() and may be extended for other cases as well.

//! \note In https://qt.gitorious.org/qt/qt/commit/bb00ac8d1251be3e703cc09e5fb2f100f24b398b,
//! a really stupid change was made:
//!
//! Don't bypass overwritten [set]data() methods in the proxy.
//!
//! By calling itemData() of the source model directly, the result
//! cannot contain data provided by the proxy model itself. The base
//! class implementation however will call data() on the proxy
//! instead.
//!
//! Changeset:
//!  QIdentityProxyModel::data(): source()->data().
//!  QIdentityProxyModel::setData(): source()->setData().
//!  -QIdentityProxyModel::itemData(): source()->itemData().
//!  -QIdentityProxyModel::setItemData(): source()->setItemData().
//!  +QIdentityProxyModel::itemData(): QAbstractItemModel::itemData().
//!  +QIdentityProxyModel::setItemData(): QAbstractItemModel::setItemData().
//!
//! Where QAbstractItemModel::itemData():
//!  for (role = 0 -> Qt::UserRole) itemdata[role] = data (role);
//! which thus now allows the proxy::data() to be overwritten and thus
//! modify the data, but breaks having any Qt::UserRole data, without
//! knowing about the roles in the proxy, which obviously is not the
//! case, as no mechanism to get all handled roles from a model exists
//! and a generic id proxy should not need to be specialized to return
//! id(). Instead of fixing the client's application to not only
//! overwrite data() and setData(), but also itemData() and
//! setItemData(), this mindless commit was made. We reverse that
//! commit and require our proxies to properly overwrite all methods.

#include <QAbstractProxyModel>
#include <QIdentityProxyModel>
#include <QSortFilterProxyModel>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        class abstract_proxy : public QAbstractProxyModel
        {
          Q_OBJECT

        public:
          abstract_proxy (QObject* = nullptr);

          //! \note Assume id() on invalid mapping, which might be wrong, but
          //! is still superior to potentially crashing.
          virtual QVariant headerData
            (int section, Qt::Orientation orientation, int role) const override;
          virtual bool setHeaderData
            (int section, Qt::Orientation, const QVariant&, int role) override;

          //! \note revert bb00ac8d1251be3e703cc09e5fb2f100f24b398b
          virtual QMap<int, QVariant> itemData (const QModelIndex&) const override;
          virtual bool setItemData
            (const QModelIndex&, const QMap<int, QVariant>&) override;
        };

        class id_proxy : public QIdentityProxyModel
        {
          Q_OBJECT

        public:
          id_proxy (QObject* = nullptr);

          //! \note No need to mapToSource(), as id(x) = x.
          virtual QVariant headerData
            (int section, Qt::Orientation orientation, int role) const override;
          virtual bool setHeaderData
            (int section, Qt::Orientation, const QVariant&, int role) override;

          //! \note revert bb00ac8d1251be3e703cc09e5fb2f100f24b398b
          virtual QMap<int, QVariant> itemData (const QModelIndex&) const override;
          virtual bool setItemData
            (const QModelIndex&, const QMap<int, QVariant>&) override;
        };

        class sort_filter_proxy : public QSortFilterProxyModel
        {
          Q_OBJECT

        public:
          sort_filter_proxy (QObject* = nullptr);

          //! \note Assume id() on invalid mapping, which might be wrong, but
          //! is still superior to potentially crashing.
          virtual QVariant headerData
            (int section, Qt::Orientation orientation, int role) const override;
          virtual bool setHeaderData
            (int section, Qt::Orientation, const QVariant&, int role) override;

          //! \note revert bb00ac8d1251be3e703cc09e5fb2f100f24b398b
          virtual QMap<int, QVariant> itemData (const QModelIndex&) const override;
          virtual bool setItemData
            (const QModelIndex&, const QMap<int, QVariant>&) override;
        };
      }
    }
  }
}
