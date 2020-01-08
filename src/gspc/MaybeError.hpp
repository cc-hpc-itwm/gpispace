#pragma once

#include <gspc/Tree.hpp>

#include <util-generic/functor_visitor.hpp>

#include <boost/variant.hpp>

#include <exception>
#include <functional>

namespace gspc
{
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
