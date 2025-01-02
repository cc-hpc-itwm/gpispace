// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <boost/optional.hpp>
#include <boost/serialization/vector.hpp>

#include <functional>
#include <iosfwd>
#include <set>
#include <vector>

#include <stdint.h>

namespace bitsetofint
{
  struct GSPC_DLLEXPORT type
  {
  public:
    explicit type (std::size_t = 0);

    void push_back (uint64_t);

    type& ins (unsigned long const&);
    type& del (unsigned long const&);
    bool is_element (unsigned long const&) const;
    std::size_t count() const;
    void list (std::ostream&) const;
    void list (std::function<void (unsigned long const&)> const&) const;
    std::set<unsigned long> elements() const;

    GSPC_DLLEXPORT friend type operator| (type const&, type const&);
    GSPC_DLLEXPORT friend type operator& (type const&, type const&);
    GSPC_DLLEXPORT friend type operator^ (type const&, type const&);

    GSPC_DLLEXPORT friend std::ostream& operator<< (std::ostream&, type const&);
    GSPC_DLLEXPORT friend std::size_t hash_value (type const&);
    GSPC_DLLEXPORT friend bool operator== (type const&, type const&);
    GSPC_DLLEXPORT friend std::string to_hex (type const&);

    GSPC_DLLEXPORT friend bool operator< (type const&, type const&);

    template<typename Archive>
      void serialize (Archive& ar, unsigned int)
    {
      ar & _container;
    }

  private:
    std::vector<uint64_t> _container;
  };

  GSPC_DLLEXPORT type operator| (type const&, type const&);
  GSPC_DLLEXPORT type operator& (type const&, type const&);
  GSPC_DLLEXPORT type operator^ (type const&, type const&);

  GSPC_DLLEXPORT bool operator== (type const&, type const&);
  GSPC_DLLEXPORT bool operator< (type const&, type const&);

  GSPC_DLLEXPORT std::size_t hash_value (type const&);

  GSPC_DLLEXPORT std::ostream& operator<< (std::ostream&, type const&);

  GSPC_DLLEXPORT std::string to_hex (type const&);
  GSPC_DLLEXPORT type from_hex (std::string const&);
  GSPC_DLLEXPORT ::boost::optional<type> from_hex
    ( std::string::const_iterator& pos
    , std::string::const_iterator const& end
    );
}
