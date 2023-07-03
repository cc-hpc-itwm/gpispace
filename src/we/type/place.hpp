// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/property.hpp>
#include <we/type/signature.hpp>

#include <boost/optional.hpp>

#include <string>

namespace place
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
    we::type::property::type const& property() const
    {
      return _prop;
    }

    //! \todo eliminate the need for the default constructor
    type () = default;
    type ( std::string const& name
         , pnet::type::signature::signature_type const& signature
         , ::boost::optional<bool> put_token
         , we::type::property::type const& prop
         )
      : _name (name)
      , _signature (signature)
      , _put_token (put_token)
      , _prop (prop)
    {}

    bool is_marked_for_put_token() const
    {
      return !!_put_token && _put_token.get();
    }

  private:
    //! \todo maybe one should factor out the (name, sig, prop)-pattern
    // into a base class
    std::string _name;
    pnet::type::signature::signature_type _signature;
    ::boost::optional<bool> _put_token;
    we::type::property::type _prop;

    friend class ::boost::serialization::access;
    template<typename Archive>
    void serialize (Archive& ar, unsigned int)
    {
      ar & _name;
      ar & _signature;
      ar & _put_token;
      ar & _prop;
    }
  };
}
