// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtCore/QtGlobal>

//! \note Qt 5.7 finally adds a nice qOverload, but we don't require
//! that. So, we just copy it here.
#if QT_VERSION < QT_VERSION_CHECK (5, 7, 0)

QT_BEGIN_NAMESPACE

template <typename... Args>
  struct QNonConstOverload
{
  template <typename R, typename T>
    Q_DECL_CONSTEXPR auto operator()(R (T::*ptr)(Args...)) const Q_DECL_NOTHROW -> decltype(ptr)
  {
    return ptr;
  }

  template <typename R, typename T>
    static Q_DECL_CONSTEXPR auto of(R (T::*ptr)(Args...)) Q_DECL_NOTHROW -> decltype(ptr)
  {
    return ptr;
  }
};

template <typename... Args>
  struct QConstOverload
{
  template <typename R, typename T>
    Q_DECL_CONSTEXPR auto operator()(R (T::*ptr)(Args...) const) const Q_DECL_NOTHROW -> decltype(ptr)
  {
    return ptr;
  }

  template <typename R, typename T>
    static Q_DECL_CONSTEXPR auto of(R (T::*ptr)(Args...) const) Q_DECL_NOTHROW -> decltype(ptr)
  {
    return ptr;
  }
};

template <typename... Args>
  struct QOverload : QConstOverload<Args...>, QNonConstOverload<Args...>
{
  using QConstOverload<Args...>::of;
  using QConstOverload<Args...>::operator();
  using QNonConstOverload<Args...>::of;
  using QNonConstOverload<Args...>::operator();

  template <typename R>
    Q_DECL_CONSTEXPR auto operator()(R (*ptr)(Args...)) const Q_DECL_NOTHROW -> decltype(ptr)
  {
    return ptr;
  }

  template <typename R>
    static Q_DECL_CONSTEXPR auto of(R (*ptr)(Args...)) Q_DECL_NOTHROW -> decltype(ptr)
  {
    return ptr;
  }
};

#if defined(__cpp_variable_templates) && __cpp_variable_templates >= 201304 // C++14
template <typename... Args>
  Q_DECL_CONSTEXPR QOverload<Args...> qOverload Q_DECL_UNUSED = {};
template <typename... Args>
  Q_DECL_CONSTEXPR QConstOverload<Args...> qConstOverload Q_DECL_UNUSED = {};
template <typename... Args>
  Q_DECL_CONSTEXPR QNonConstOverload<Args...> qNonConstOverload Q_DECL_UNUSED = {};
#endif

QT_END_NAMESPACE

#endif
