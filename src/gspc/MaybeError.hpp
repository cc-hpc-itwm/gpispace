#pragma once

#include <gspc/Tree.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/callable_signature.hpp>

#include <boost/variant.hpp>

#include <exception>
#include <functional>
#include <type_traits>

namespace gspc
{
  template<typename T>
    struct ErrorOr : private boost::variant<std::exception_ptr, T>
  {
    ErrorOr (T) noexcept;
    ErrorOr (std::exception_ptr) noexcept;

    template
      < typename Function
      , typename = std::enable_if_t<fhg::util::is_callable<Function, T()>{}>
      >
      ErrorOr (Function&&) noexcept;

    explicit operator bool() const noexcept;

    // monad::bind
    template
      < typename Function
      , typename U = fhg::util::return_type<Function>
      , typename = std::enable_if_t<fhg::util::is_callable<Function, U (T)>{}>
      >
      ErrorOr<U> operator>> (Function&&) && noexcept;
  };

  // std::unordered_map<K, MaybeError<U>> operator>>
  //   ( std::unordered_map<K, MaybeError<T>>
  //   , Function
  //   )
  // {}

  template<typename T>
    using MaybeError = boost::variant<std::exception_ptr, T>;

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

#include <util-generic/serialization/exception.hpp>
#include <boost/serialization/variant.hpp>

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void serialize (Archive& ar, std::exception_ptr ptr, unsigned int)
    {
      std::string s;
      if (typename Archive::is_saving{})
      {
        s = fhg::util::serialization::exception::serialize (ptr);
      }
      ar & s;
      if (typename Archive::is_loading{})
      {
        ptr = fhg::util::serialization::exception::deserialize (s);
      }
    }
  }
}
