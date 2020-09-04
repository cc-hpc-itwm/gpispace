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

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      // The context given to async_x is used as a tag type to deduce
      // a "handler", and that handler has an "async result".
      // We wrap the handler for boost::asio::yield_context (not
      // basic_yield_context to avoid one level of Handler indirection
      // overhead, we currently don't need that). The implementation
      // of that is in `boost::asio::detail::`, so we don't refer to
      // it directly but deduce it with the `handler_type` trait
      // class. We are inheriting from those detail classes to reuse
      // their boilerplate implementation as well as the actual
      // coroutine logic.
      // Relevant to this wrapper, the handler's operator() yields and
      // the async_result's get() resumes the coroutine. By calling
      // the user given function before either we can nicely hook that
      // point in time. The remainder of this is
      // boilerplate. Boost.Asio has a *lot* of boilerplate.

      template<typename Ret, typename... Args>
        using coro_handler_for
          = typename boost::asio::handler_type<boost::asio::yield_context, Ret (Args...)>::type;

      template<typename Ret, typename... Args>
        struct coro_handler_with_hooks : coro_handler_for<Ret, Args...>
      {
        using coro_handler = coro_handler_for<Ret, Args...>;
        using coro_async_result = boost::asio::async_result<coro_handler>;

        std::function<void()> before_yield;
        std::function<void()> before_resume;

        coro_handler_with_hooks (yield_context_with_hooks context)
          : coro_handler (std::move (context.yield_context))
          , before_yield (std::move (context.before_yield))
          , before_resume (std::move (context.before_resume))
        {}

        auto operator() (Args...  args)
          -> decltype (std::declval<coro_handler&>() (args...))
        {
          before_resume();
          return coro_handler::operator() (args...);
        }

        struct async_result : coro_async_result
        {
          // Do *not* keep a reference to handler instead, their
          // lifetime is shorter (even though they require to be
          // passed by reference...)!
          std::function<void()> before_yield;

          async_result (coro_handler_with_hooks& handler)
            : coro_async_result (handler)
            , before_yield (handler.before_yield)
          {}

          auto get() -> decltype (std::declval<coro_async_result&>().get())
          {
            before_yield();
            return coro_async_result::get();
          }
        };
      };
    }
  }
}

namespace boost
{
  namespace asio
  {
    namespace rpcd = fhg::rpc::detail;

    template<typename Ret, typename... Args>
      struct handler_type<rpcd::yield_context_with_hooks, Ret (Args...)>
    {
      using type = rpcd::coro_handler_with_hooks<Ret, Args...>;
    };

    template<typename Ret, typename... Args>
      struct async_result<rpcd::coro_handler_with_hooks<Ret, Args...>>
        : rpcd::coro_handler_with_hooks<Ret, Args...>::async_result
    {
      using rpcd::coro_handler_with_hooks<Ret, Args...>::async_result::async_result;
    };
  }
}
