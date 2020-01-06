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

  // monad::bind
  template<typename T, typename U>
    MaybeError<U> bind
    ( MaybeError<T> const& x
    , std::function<U (T const&)> const& function
    ) noexcept;

  // error handling in trees
  template<typename T>
    using TreeResult = Tree<MaybeError<T>>;

  template<typename T> TreeResult<T> mreturn (Tree<T> const&);

  template<typename T, typename U>
    TreeResult<U> bind ( TreeResult<T> const&
                       , std::function<U (T const&)> const&
                       ) noexcept;
}

#include <gspc/MaybeError.ipp>
