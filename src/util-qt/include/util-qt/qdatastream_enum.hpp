// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
