#pragma once

#include <gspc/Tree.hpp>

#include <util-generic/functor_visitor.hpp>

#include <boost/variant.hpp>

#include <exception>
#include <functional>

namespace gspc
{
  template<typename T = void>
    using MaybeError = boost::variant<T, std::exception_ptr>;

  template<typename T> bool is_success (MaybeError<T> const&);
  template<typename T> bool is_failure (MaybeError<T> const&);

  // monad::return
  template<typename T> MaybeError<T> mreturn (T) noexcept;
  template<typename T> MaybeError<T> mreturn (std::exception_ptr) noexcept;
  template<typename Fun>
    MaybeError<decltype(std::declval<Fun>()())> bind
    ( Fun&& function
    ) noexcept;
  //! \todo use struct and let mreturn and bind (Fun) be ctors

  // monad::bind
  template<typename T, typename Fun>
    MaybeError<decltype(std::declval<Fun>()(std::declval<T>()))> bind
    ( MaybeError<T>&& x
    , Fun&& function
    ) noexcept;
  //! \todo operator>>
  //! \todo overloads for containers
}

#include <gspc/MaybeError.ipp>
