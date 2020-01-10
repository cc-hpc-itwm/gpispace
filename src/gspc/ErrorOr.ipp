#include <util-generic/functor_visitor.hpp>
#include <util-generic/serialization/boost/blank.hpp>

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
