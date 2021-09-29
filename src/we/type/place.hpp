// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <we/type/signature.hpp>
#include <we/type/property.hpp>

#include <boost/optional.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>

namespace place
{
  //! \todo add properties
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
         , boost::optional<bool> put_token
         , we::type::property::type const& prop = we::type::property::type ()
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
    boost::optional<bool> _put_token;
    we::type::property::type _prop;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive& ar, unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_name);
      ar & BOOST_SERIALIZATION_NVP(_signature);
      ar & BOOST_SERIALIZATION_NVP(_put_token);
      ar & BOOST_SERIALIZATION_NVP(_prop);
    }
  };
}
