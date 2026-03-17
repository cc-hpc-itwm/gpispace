// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

// This file is included at the end of shared.hpp
// It requires value_type to be complete (include value.hpp before using)

#include <gspc/we/type/value.hpp>

namespace gspc::pnet::type::value
{
  struct value_type_wrapper
  {
    value_type_wrapper (value_type value)
      : _value {std::move (value)}
    {}

    template<typename Archive>
      void serialize (Archive& ar, unsigned int)
    {
      ar & _value;
    }

    [[nodiscard]] constexpr auto value() const noexcept -> value_type const&
    {
      return _value;
    }

    value_type_wrapper() = default; // \note serialization only

    value_type _value;
  };
}

namespace gspc::we::type
{
  inline shared::shared()
    : _value {nullptr}
    , _cleanup_place {}
  {}

  template<typename T>
    shared::shared (T value, std::string cleanup_place)
      : _value {new pnet::type::value::value_type_wrapper {std::move (value)}}
      , _cleanup_place {std::move (cleanup_place)}
  {}

  inline shared::~shared()
  {
    delete _value;
  }

  inline shared::shared (shared const& other)
    : _value {other._value ? new pnet::type::value::value_type_wrapper {other._value->_value} : nullptr}
    , _cleanup_place {other._cleanup_place}
  {}

  inline shared& shared::operator= (shared const& other)
  {
    if (this != &other)
    {
      delete _value;
      _value = other._value ? new pnet::type::value::value_type_wrapper {other._value->_value} : nullptr;
      _cleanup_place = other._cleanup_place;
    }
    return *this;
  }

  inline shared::shared (shared&& other) noexcept
    : _value {other._value}
    , _cleanup_place {std::move (other._cleanup_place)}
  {
    other._value = nullptr;
  }

  inline shared& shared::operator= (shared&& other) noexcept
  {
    if (this != &other)
    {
      delete _value;
      _value = other._value;
      _cleanup_place = std::move (other._cleanup_place);
      other._value = nullptr;
    }
    return *this;
  }

  inline auto shared::value() const -> decltype (auto)
  {
    if (!_value)
    {
      throw std::logic_error
        { "shared::value(): accessing uninitialized shared value"
        };
    }

    return _value->value();
  }

  template<typename Archive>
    void shared::serialize (Archive& ar, unsigned int)
  {
    ar & _value;
    ar & _cleanup_place;
  }
}
