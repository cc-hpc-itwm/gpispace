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
  template<typename T, typename U>
    MaybeError<U> bind
    ( MaybeError<T> const& x
    , std::function<U (T const&)> const& function
    ) noexcept
  {
    return fhg::util::visit<MaybeError<U>>
      ( x
      , [] (std::exception_ptr const& exception) -> MaybeError<U>
        {
          return exception;
        }
      , [&] (T const& value) -> MaybeError<U>
        {
          try
          {
            return function (value);
          }
          catch (...)
          {
            return std::current_exception();
          }
        }
      );
  }

  template<typename T>
    TreeResult<T> mreturn (Tree<T> const& tree)
  {
    return apply<T, MaybeError<T>>
      ( tree
      , [&] (T const& x)
        {
          return mreturn (x);
        }
      );
  }

  template<typename T, typename U>
    TreeResult<U> bind ( TreeResult<T> const& tree
                       , std::function<U (T const&)> const& function
                       ) noexcept
  {
    return apply<MaybeError<T>, MaybeError<U>>
      ( tree
      , [&] (MaybeError<T> const& x)
        {
          return bind<T, U> (x, function);
        }
      );
  }
}
