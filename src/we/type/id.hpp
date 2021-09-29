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

#include <cstdint>
#include <iostream>

#include <boost/serialization/nvp.hpp>
#include <boost/functional/hash.hpp>

#if __cplusplus >= 201103L
#include <functional>
#endif

namespace we
{
  template<typename POD_TYPE, class client_type>
  struct id_base_type
  {
    typedef POD_TYPE pod_type;

    id_base_type ()
      : _value (0)
    {}

    explicit
    id_base_type (pod_type const& value)
      : _value (value)
    {}

    bool operator== (id_base_type const& other) const
    {
      return _value == other._value;
    }
    bool operator!= (id_base_type const& other) const
    {
      return _value != other._value;
    }
    bool operator< (id_base_type const& other) const
    {
      return _value < other._value;
    }
    bool operator> (id_base_type const& other) const
    {
      return _value > other._value;
    }
    client_type operator++ (int)
    {
      const client_type old (_value);

      ++_value;

      return old;
    }

    pod_type const& value () const
    {
      return _value;
    }

    template<typename P, typename C>
    friend std::ostream& operator<< (std::ostream&, id_base_type<P, C> const&);
    template<typename P, typename C>
    friend std::istream& operator>> (std::istream&, id_base_type<P, C>&);
    template<typename P, typename C>
    friend std::size_t hash_value (id_base_type<P, C> const&);
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize (Archive& ar, unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_value);
    }

  private:
    pod_type _value;
  };

  template<typename pod_type, typename client_type>
  std::size_t hash_value (id_base_type<pod_type, client_type> const& id)
  {
    return boost::hash<pod_type>() (id._value);
  }

  template<typename pod_type, typename client_type>
  std::ostream& operator<< ( std::ostream& s
                           , id_base_type<pod_type, client_type> const& id
                           )
  {
    return s << id._value;
  }

  template<typename pod_type, typename client_type>
  std::istream& operator>> ( std::istream& i
                           , id_base_type<pod_type, client_type>& id
                           )
  {
    return i >> id._value;
  }
}

#if __cplusplus >= 201103L
#define INHERIT_ID_TYPE(_name,_type)                                    \
  namespace we                                                          \
  {                                                                     \
    struct _name : public ::we::id_base_type<_type, _name>              \
    {                                                                   \
      _name () : id_base_type<_type,_name>() {}                         \
      _name (_type const& value) : id_base_type<_type,_name> (value) {} \
    };                                                                  \
  }                                                                     \
  namespace std                                                         \
  {                                                                     \
    template<> struct hash<we::_name>                                   \
    {                                                                   \
      size_t operator() (we::_name const& id) const                     \
      {                                                                 \
        return std::hash<_type>() (id.value());                         \
      }                                                                 \
    };                                                                  \
  }
#else
#define INHERIT_ID_TYPE(_name,_type)                                    \
  namespace we                                                          \
  {                                                                     \
    struct _name : public ::we::id_base_type<_type, _name>              \
    {                                                                   \
      _name () : id_base_type<_type,_name>() {}                         \
      _name (_type const& value) : id_base_type<_type,_name> (value) {} \
    };                                                                  \
  }
#endif

  // Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
  // cycles per second, you can run for 2^64/3e9/60/60/24/365 > 194 years.
  // It follows that an uint64_t is enough for now.

  INHERIT_ID_TYPE (place_id_type, std::uint64_t)
  INHERIT_ID_TYPE (port_id_type, std::uint64_t)
  INHERIT_ID_TYPE (transition_id_type, std::uint64_t)
  INHERIT_ID_TYPE (token_id_type, std::uint64_t)

  INHERIT_ID_TYPE (priority_type, std::int16_t)

#undef INHERIT_ID_TYPE
