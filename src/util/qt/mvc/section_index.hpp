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

#include <QHeaderView>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        struct section_index
        {
          QAbstractItemModel* _model;
          Qt::Orientation _orientation;
          int _section;

          //! \note Required for Q_DECLARE_METATYPE.
          section_index();
          section_index (const QAbstractItemModel*, Qt::Orientation, int section);
          section_index (const QHeaderView*, int section);
          section_index (QModelIndex, Qt::Orientation);

          QVariant data (int role = Qt::DisplayRole) const;
          bool data (QVariant, int role = Qt::EditRole) const;

          bool operator< (section_index const&) const;
          bool operator== (section_index const&) const;
        };

        QDebug operator<< (QDebug, section_index);
        size_t hash_value (section_index);
        uint qHash (section_index);
      }
    }
  }
}

Q_DECLARE_METATYPE (fhg::util::qt::mvc::section_index)
