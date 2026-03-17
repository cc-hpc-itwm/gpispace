namespace fmt
{
  template<typename T>
    template<typename ParseContext>
      constexpr auto
        formatter<std::optional<T>>::parse
          (ParseContext& context)
  {
    static_assert (is_formattable<T>::value);

    return context.begin();
  }

  template<typename T>
    template<typename FormatContext>
      constexpr auto
        formatter<std::optional<T>>::format
          ( std::optional<T> const& optional
          , FormatContext& context
          ) const
            -> decltype (context.out())
  {
    return optional.has_value()
      ? fmt::format_to (context.out(), "Just {}", *optional)
      : fmt::format_to (context.out(), "Nothing")
      ;
  }
}

#include <fmt/ostream.h>

namespace std
{
  template<typename T>
    auto operator<< (ostream& os, optional<T> const& optional) -> ostream&
  {
    fmt::print (os, "{}", optional);

    return os;
  }
}
