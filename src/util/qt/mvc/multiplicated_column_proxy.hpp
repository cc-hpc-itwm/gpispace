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

#include <util/qt/mvc/fixed_proxy_models.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        class multiplicated_column_proxy : public id_proxy
        {
          Q_OBJECT

        public:
          multiplicated_column_proxy (QAbstractItemModel*, QObject* parent = nullptr);

          virtual int columnCount (const QModelIndex& = QModelIndex()) const override;
          virtual QModelIndex mapToSource (const QModelIndex& proxy) const override;
          virtual bool insertColumns
            (int column, int count, const QModelIndex& parent = QModelIndex()) override;
          virtual bool removeColumns
            (int column, int count, const QModelIndex& parent = QModelIndex()) override;

        private slots:
          void source_dataChanged (const QModelIndex&, const QModelIndex&);

        private:
          int _column_count;
        };
      }
    }
  }
}
