// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util/qt/mvc/section_index.hpp>

#include <boost/functional/hash.hpp>

#include <QDebug>

#include <tuple>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        section_index::section_index()
          : _model (nullptr)
          , _orientation (Qt::Horizontal)
          , _section (-1)
        { }

        section_index::section_index ( const QAbstractItemModel* model
                                     , Qt::Orientation orientation
                                     , int section
                                     )
          : _model (const_cast<QAbstractItemModel*> (model))
          , _orientation (orientation)
          , _section (section)
        { }

        section_index::section_index (const QHeaderView* view, int section)
          : _model (view->model())
          , _orientation (view->orientation())
          , _section (section)
        { }

        section_index::section_index (QModelIndex index, Qt::Orientation orientation)
          : _model (const_cast<QAbstractItemModel*> (index.model()))
          , _orientation (orientation)
          , _section (_orientation == Qt::Horizontal ? index.column() : index.row())
        { }

        QVariant section_index::data (int role) const
        {
          return _model->headerData (_section, _orientation, role);
        }
        bool section_index::data (QVariant val, int role) const
        {
          return _model->setHeaderData (_section, _orientation, val, role);
        }

        bool section_index::operator< (const section_index& other) const
        {
          return std::tie (_model, _orientation, _section)
               < std::tie (other._model, other._orientation, other._section);
        }
        bool section_index::operator== (const section_index& other) const
        {
          return std::tie (_model, _orientation, _section)
              == std::tie (other._model, other._orientation, other._section);
        }

        QDebug operator<< (QDebug d, section_index index)
        {
          return d << index._model << index._orientation << index._section;
        }

        size_t hash_value (section_index index)
        {
          size_t seed (0);
          boost::hash_combine (seed, index._model);
          boost::hash_combine (seed, index._orientation);
          boost::hash_combine (seed, index._section);
          return seed;
        }

        uint qHash (section_index index)
        {
          return static_cast<uint> (hash_value (index));
        }
      }
    }
  }
}
