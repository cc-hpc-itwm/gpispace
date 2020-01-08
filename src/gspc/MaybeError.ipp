#include <util-generic/functor_visitor.hpp>

#include <utility>

namespace gspc
{
  template<typename T> bool is_success (MaybeError<T> const& x)
  {
    return fhg::util::visit<bool>
      ( x
      , [] (std::exception_ptr const&) { return false; }
      , [] (T const&) { return true; }
      );
  }
  template<typename T> bool is_failure (MaybeError<T> const& x)
  {
    return !is_success (x);
  }

  template<typename T>
    MaybeError<T> mreturn (T x) noexcept
  {
    return std::move (x);
  }
  template<typename T>
    MaybeError<T> mreturn (std::exception_ptr exception) noexcept
  {
    return std::move (exception);
  }
  template<typename Fun>
    MaybeError<decltype(std::declval<Fun>()())> bind
    ( Fun&& function
    ) noexcept
  {
    try
    {
      return std::move (function)();
    }
    catch (...)
    {
      return std::current_exception();
    }
  }

  template<typename T, typename Fun>
    MaybeError<decltype (std::declval<Fun>()(std::declval<T>()))> bind
    ( MaybeError<T>&& x
    , Fun&& function
    ) noexcept
  {
    using U = decltype (std::declval<Fun>()(std::declval<T>()));

    return fhg::util::visit<MaybeError<U>>
      ( x
      , [] (std::exception_ptr exception) -> MaybeError<U>
        {
          return std::move (exception);
        }
      , [&] (T value) -> MaybeError<U>
        {
          return bind ([&] { return function (std::move (value)); });
        }
      );
  }
}
