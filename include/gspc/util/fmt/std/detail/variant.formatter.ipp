namespace fmt
{
  template<typename... Ts>
    template<typename ParseContext>
      constexpr auto
        formatter<std::variant<Ts...>>::parse
          (ParseContext& context)
  {
    static_assert ((is_formattable<Ts>::value && ...));

    return context.begin();
  }

  template<typename... Ts>
    template<typename FormatContext>
      constexpr auto
        formatter<std::variant<Ts...>>::format
          ( std::variant<Ts...> const& variant
          , FormatContext& context
          ) const
            -> decltype (context.out())
  {
    return std::visit
      ( [&] (auto const& value)
        {
          return fmt::format_to (context.out(), "{}", value);
        }
      , variant
      );
  }
}

#include <fmt/ostream.h>

namespace std
{
  template<typename... Ts>
    auto operator<< (ostream& os, variant<Ts...> const& variant) -> ostream&
  {
    fmt::print (os, "{}", variant);

    return os;
  }
}
