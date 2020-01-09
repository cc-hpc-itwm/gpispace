#pragma once

#include <util-generic/callable_signature.hpp>

#include <boost/variant.hpp>

#include <type_traits>

namespace gspc
{
  template<typename T>
    struct ErrorOr : /* private */ boost::variant<std::exception_ptr, T>
  {
    ErrorOr (T) noexcept;
    ErrorOr (std::exception_ptr) noexcept;

    template
      < typename Function
      //      , typename = std::enable_if_t<fhg::util::is_callable<Function, T()>{}>
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

    // unbind
    T const& value() const;
    std::exception_ptr error() const;

    // serialization
    ErrorOr() = default;

    template<typename Archive>
      void serialize (Archive& ar, unsigned int)
    {
      ar & *this;
    }

  private:
    using Base = boost::variant<std::exception_ptr, T>;
  };
}

#include <unordered_map>

namespace gspc
{
  template
    < typename Key
    , typename T
    , typename Function
    , typename U = fhg::util::return_type<Function>
    , typename = std::enable_if<fhg::util::is_callable<Function, U (T)>{}>
    >
    std::unordered_map<Key, ErrorOr<U>> operator>>
      ( std::unordered_map<Key, ErrorOr<T>>&&
      , Function&&
      );
}

#include <gspc/ErrorOr.ipp>

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
