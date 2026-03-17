// Copyright (C) 2010-2011,2013,2015-2016,2019,2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <optional>
#include <boost/serialization/vector.hpp>

#include <functional>
#include <iosfwd>
#include <optional>
#include <set>
#include <vector>

#include <stdint.h>

namespace gspc::pnet::type::bitsetofint
{
  struct GSPC_EXPORT type
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

    GSPC_EXPORT friend type operator| (type const&, type const&);
    GSPC_EXPORT friend type operator& (type const&, type const&);
    GSPC_EXPORT friend type operator^ (type const&, type const&);

    GSPC_EXPORT friend std::ostream& operator<< (std::ostream&, type const&);
    GSPC_EXPORT friend std::size_t hash_value (type const&);
    GSPC_EXPORT friend bool operator== (type const&, type const&);
    GSPC_EXPORT friend std::string to_hex (type const&);

    GSPC_EXPORT friend bool operator< (type const&, type const&);

    template<typename Archive>
      void serialize (Archive& ar, unsigned int)
    {
      ar & _container;
    }

  private:
    std::vector<uint64_t> _container;
  };

  GSPC_EXPORT type operator| (type const&, type const&);
  GSPC_EXPORT type operator& (type const&, type const&);
  GSPC_EXPORT type operator^ (type const&, type const&);

  GSPC_EXPORT bool operator== (type const&, type const&);
  GSPC_EXPORT bool operator< (type const&, type const&);

  GSPC_EXPORT std::size_t hash_value (type const&);

  GSPC_EXPORT std::ostream& operator<< (std::ostream&, type const&);

  GSPC_EXPORT std::string to_hex (type const&);
  GSPC_EXPORT type from_hex (std::string const&);
  GSPC_EXPORT std::optional<type> from_hex
    ( std::string::const_iterator& pos
    , std::string::const_iterator const& end
    );
}
