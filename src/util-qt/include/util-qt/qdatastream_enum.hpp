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

#include <QtCore/QDataStream>

#include <type_traits>

#if QT_VERSION < QT_VERSION_CHECK (5, 14, 0)

//! \note Does not use fhg::util::qt:: in order to be eligible for ADL.

QT_BEGIN_NAMESPACE

template < typename T
         , typename = typename std::enable_if<std::is_enum<T>{}>::type
         >
  QDataStream& operator<< (QDataStream& stream, T x)
{
  stream << static_cast<typename std::underlying_type<T>::type> (x);
  return stream;
}

template < typename T
         , typename = typename std::enable_if<std::is_enum<T>{}>::type
         >
  QDataStream& operator>> (QDataStream& stream, T& x)
{
  typename std::underlying_type<T>::type raw;
  stream >> raw;
  x = static_cast<T> (raw);
  return stream;
}

QT_END_NAMESPACE

#endif
