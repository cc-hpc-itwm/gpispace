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
  GSPC_DLLEXPORT boost::optional<type> from_hex
    ( std::string::const_iterator& pos
    , std::string::const_iterator const& end
    );
}
