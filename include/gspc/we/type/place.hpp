// Copyright (C) 2010,2012-2016,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/property.hpp>
#include <gspc/we/type/signature.hpp>
#include <gspc/util/serialization/std/variant.hpp>

#include <optional>

#include <string>
#include <variant>

namespace gspc::we::type::place
{
  struct type
  {
  public:
    std::string const& name() const
    {
      return _name;
    }

    pnet::type::signature::signature_type const& signature() const
    {
      return _signature;
    }
    property::type const& property() const
    {
      return _prop;
    }

    //! \todo eliminate the need for the default constructor
    type () = default;

    struct Generator
    {
      struct Yes
      {
        template<typename Archive>
          void serialize (Archive&, unsigned int)
        {
          // empty
        }
      };
      struct No
      {
        template<typename Archive>
          void serialize (Archive&, unsigned int)
        {
          // empty
        }
      };
    };

    type ( std::string const& name
         , pnet::type::signature::signature_type const& signature
         , std::optional<bool> put_token
         , std::optional<bool> shared_sink
          , property::type const& prop
         , std::variant<Generator::Yes, Generator::No> generator
         )
      : _name {name}
      , _signature {signature}
      , _put_token {put_token}
      , _shared_sink {shared_sink}
      , _generator {generator}
      , _prop {prop}
    {}

    bool is_marked_for_put_token() const
    {
      return !!_put_token && _put_token.value();
    }

    bool is_shared_sink() const
    {
      return _shared_sink.value_or (false);
    }

    bool is_generator() const
    {
      return std::holds_alternative<Generator::Yes> (_generator);
    }

  private:
    //! \todo maybe one should factor out the (name, sig, prop)-pattern
    // into a base class
    std::string _name;
    pnet::type::signature::signature_type _signature;
    std::optional<bool> _put_token;
    std::optional<bool> _shared_sink;
    std::variant<Generator::Yes, Generator::No> _generator;
    property::type _prop;

    friend class ::boost::serialization::access;
    template<typename Archive>
    void serialize (Archive& ar, unsigned int)
    {
      ar & _name;
      ar & _signature;
      ar & _put_token;
      ar & _shared_sink;
      ar & _generator;
      ar & _prop;
    }
  };
}
