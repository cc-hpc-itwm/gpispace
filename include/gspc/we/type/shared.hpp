// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/type/signature.hpp>

#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>

namespace gspc::pnet::type::value
{
  // Forward declaration to break cyclic dependency with value_type.
  struct value_type_wrapper;
}

namespace gspc::we::type
{
  // A value type wrapper for garbage collection.
  // The cleanup place name indicates where a cleanup token should be
  // placed when the value is no longer needed (tracked in net.cpp).
  //
  // The shared value can be any gspc::pnet::type::value::value_type.
  //
  class GSPC_EXPORT shared
  {
  public:
    // Construct with any value type
    template<typename T>
      shared (T value, std::string cleanup_place);

    ~shared();
    shared (shared const&);
    shared& operator= (shared const&);
    shared (shared&&) noexcept;
    shared& operator= (shared&&) noexcept;

    // Access the stored value - returns gspc::pnet::type::value::value_type const&
    // Defined in shared.ipp after value_type is complete
    [[nodiscard]] auto value() const -> decltype(auto);

    [[nodiscard]] auto cleanup_place() const -> std::string const&
    {
      return _cleanup_place;
    }

    // Check if a type name is a shared type (shared_PLACENAME).
    // If so, return the cleanup place name, otherwise return nullopt.
    //
    GSPC_EXPORT
      static std::optional<std::string> cleanup_place (std::string const&);

    GSPC_EXPORT
      friend std::ostream& operator<< (std::ostream&, shared const&);
    GSPC_EXPORT
      friend std::size_t hash_value (shared const&);
    GSPC_EXPORT
      friend bool operator== (shared const&, shared const&);
    GSPC_EXPORT
      friend bool operator!= (shared const&, shared const&);
    GSPC_EXPORT
      friend bool operator< (shared const&, shared const&);
    GSPC_EXPORT
      friend bool operator<= (shared const&, shared const&);
    GSPC_EXPORT
      friend bool operator> (shared const&, shared const&);
    GSPC_EXPORT
      friend bool operator>= (shared const&, shared const&);

    shared(); //! \note serialization only
    template<typename Archive>
      void serialize (Archive& ar, unsigned int);

  private:
    pnet::type::value::value_type_wrapper* _value {nullptr};
    std::string _cleanup_place;
  };
}

// Template implementations need value_type to be complete
#include <gspc/we/type/shared.ipp>
