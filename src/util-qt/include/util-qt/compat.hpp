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

#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>

QT_BEGIN_NAMESPACE

template<typename T>
  inline QSet<T> list_to_set (QList<T> const& list)
{
#if QT_VERSION < QT_VERSION_CHECK (5, 14, 0)
  return list.toSet();
#else
  return QSet<T> (list.begin(), list.end());
#endif
}

template<typename T>
  inline QList<T> set_to_list (QSet<T> const& set)
{
#if QT_VERSION < QT_VERSION_CHECK (5, 14, 0)
  return QList<T>::fromSet (set);
#else
  return QList<T> (set.begin(), set.end());
#endif
}

inline int horizontal_advance (QFont const& font, QString const& text)
{
  QFontMetrics const metrics (font);
#if QT_VERSION < QT_VERSION_CHECK (5, 11, 0)
  return metrics.width (text);
#else
  return metrics.horizontalAdvance (text);
#endif
}

QT_END_NAMESPACE
