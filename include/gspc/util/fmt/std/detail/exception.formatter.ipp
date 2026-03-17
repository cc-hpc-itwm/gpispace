// Copyright (C) 2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/fmt/std/exception.formatter.hpp>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto
      formatter<std::exception>::parse
        (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto
      formatter<std::exception>::format
        ( std::exception const& error
        , FormatContext& ctx
        ) const -> decltype (ctx.out())
  {
    fmt::format_to (ctx.out(), "{}", error.what());

    try
    {
      std::rethrow_if_nested (error);
    }
    catch (std::exception const& rethrown_exception)
    {
      fmt::format_to (ctx.out(), ": {}", rethrown_exception);
    }
    catch (...)
    {
      fmt::format_to (ctx.out(), "UNKNOWN");
    }

    return ctx.out();
  }

  template<typename ParseContext>
    constexpr auto
      formatter<std::exception_ptr>::parse
        (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto
      formatter<std::exception_ptr>::format
        ( std::exception_ptr const& error
        , FormatContext& ctx
        ) const -> decltype (ctx.out())
  {
    try
    {
      std::rethrow_exception (error);
    }
    catch (std::exception const& rethrown_exception)
    {
      fmt::format_to (ctx.out(), "{}", rethrown_exception);
    }
    catch (...)
    {
      fmt::format_to (ctx.out(), "UNKNOWN");
    }

    return ctx.out();
  }
}
