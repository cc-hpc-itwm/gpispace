// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdexcept>
#include <variant>

namespace fhg::util
{
  auto variant_index_out_of_bound
    ( std::size_t i
    , std::size_t N
    ) -> std::logic_error
    ;
}

namespace boost::serialization
{
  namespace
  {
    template<std::size_t I, typename Archive, typename Variant>
      constexpr auto from_index
        ( Archive& ar
        , Variant& variant
        , std::size_t index
        ) -> void
    {
      if constexpr (! (I < std::variant_size_v<Variant>))
      {
        throw fhg::util::variant_index_out_of_bound
          (I, std::variant_size_v<Variant>);
      }
      else if (index == 0)
      {
        auto x {std::variant_alternative_t<I, Variant>{}};
        ar >> x;
        variant = x;
      }
      else
      {
        return from_index<I + 1> (ar, variant, index - 1);
      }
    }
  }

  template<typename Archive, typename... Ts>
    void load
      (Archive& ar, std::variant<Ts...>& variant, const unsigned int)
  {
    auto index {decltype (variant.index()){}};
    ar >> index;
    from_index<0> (ar, variant, index);
  }
  template<typename Archive, typename... Ts>
    void save
      (Archive& ar, std::variant<Ts...> const& variant, const unsigned int)
  {
    ar << variant.index();

    std::visit
      ( [&] (auto const& x)
        {
          ar << x;
        }
      , variant
      );
  }

  template<typename Archive, typename... Ts>
    void serialize
     ( Archive& ar
     , std::variant<Ts...>& variant
     , const unsigned int version
     )
  {
    split_free (ar, variant, version);
  }
}
