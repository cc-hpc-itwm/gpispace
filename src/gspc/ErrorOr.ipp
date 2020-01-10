#include <util-generic/functor_visitor.hpp>
#include <util-generic/hash/boost/variant.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/serialization/boost/blank.hpp>

#include <cstdint>
#include <exception>
#include <functional>

namespace gspc
{
  template<typename T>
    ErrorOr<T>::ErrorOr (T x) noexcept
      : Base {std::move (x)}
  {}
  template<typename T>
    ErrorOr<T>::ErrorOr (std::exception_ptr exception) noexcept
      : Base {std::move (exception)}
  {}

  template<typename T>
    T const& ErrorOr<T>::value() const
  {
    //! \todo specific exception
    return boost::get<T> (*this);
  }

  template<typename T>
    std::exception_ptr ErrorOr<T>::error() const
  {
    //! \todo specific exception
    return boost::get<std::exception_ptr> (*this);
  }

  template<typename T>
    template<typename Function, typename>
      ErrorOr<T>::ErrorOr (Function&& function) noexcept
        : Base
          { [&]() -> Base
            {
              static_assert
                (fhg::util::is_callable<Function, T()>{}, "fun not T()");

              try
              {
                return std::move (function)();
              }
              catch (...)
              {
                return std::current_exception();
              }
            }()
          }
  {}

  template<typename T>
    ErrorOr<T>::operator bool() const noexcept
  {
    return fhg::util::visit<bool>
      ( static_cast<Base const&> (*this)
      , [] (std::exception_ptr const&) { return false; }
      , [] (T const&) { return true; }
      );
  }

  template<typename U>
    bool operator== (ErrorOr<U> const& lhs, ErrorOr<U> const& rhs)
  {
    return static_cast<typename ErrorOr<U>::Base const&> (lhs)
      == static_cast<typename ErrorOr<U>::Base const&> (rhs);
  }

  template<typename U>
    std::ostream& operator<< (std::ostream& os, ErrorOr<U> const& x)
  {
    if (x)
    {
      return os << "Just: " << x.value();
    }
    else
    {
      return os << fhg::util::exception_printer (x.error());
    }
  }

  template<typename T>
    template<typename Function, typename U, typename>
    ErrorOr<U> ErrorOr<T>::operator>>= (Function&& function) && noexcept
  {
    return fhg::util::visit<ErrorOr<U>>
      ( static_cast<Base&&> (*this)
      , [] (std::exception_ptr exception) -> ErrorOr<U>
        {
          return std::move (exception);
        }
      , [&] (T value) -> ErrorOr<U>
        {
          return std::move (function) (std::move (value));
        }
      );
  }

  template<typename Function, typename U, typename Key, typename T, typename>
    std::unordered_map<Key, ErrorOr<U>> operator>>=
      ( std::unordered_map<Key, ErrorOr<T>>&& xs
      , Function&& function
      )
  {
    std::unordered_map<Key, ErrorOr<U>> ys;

    for (auto&& x : xs)
    {
      ys.emplace
        ( x.first
        , std::move (x.second)
          >>= [&] (T z) { return function (x.first, std::move (z)); }
        );
    }

    return ys;
  }
}

namespace std
{
  template<>
    struct hash<exception_ptr>
  {
    std::size_t operator() (exception_ptr const&) const
    {
      //! \todo this is a highly low quality hash. throw and compare
      //! catched address?! remove and replace with own
      //! exception/error infrastructure?!
      return 0;
    }
  };

  template<typename T>
    struct hash<gspc::ErrorOr<T>>
  {
    std::size_t operator() (gspc::ErrorOr<T> const& eo) const
    {
      return std::hash<typename gspc::ErrorOr<T>::Base>{} (eo);
    }
  };
}
