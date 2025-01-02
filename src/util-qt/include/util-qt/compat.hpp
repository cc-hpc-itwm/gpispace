// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
