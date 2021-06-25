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
    explicit type (const std::size_t = 0);

    void push_back (uint64_t);

    type& ins (const unsigned long&);
    type& del (const unsigned long&);
    bool is_element (const unsigned long&) const;
    std::size_t count() const;
    void list (std::ostream&) const;
    void list (const std::function<void (const unsigned long&)>&) const;
    std::set<unsigned long> elements() const;

    GSPC_DLLEXPORT friend type operator| (const type&, const type&);
    GSPC_DLLEXPORT friend type operator& (const type&, const type&);
    GSPC_DLLEXPORT friend type operator^ (const type&, const type&);

    GSPC_DLLEXPORT friend std::ostream& operator<< (std::ostream&, const type&);
    GSPC_DLLEXPORT friend std::size_t hash_value (const type&);
    GSPC_DLLEXPORT friend bool operator== (const type&, const type&);
    GSPC_DLLEXPORT friend std::string to_hex (const type&);

    GSPC_DLLEXPORT friend bool operator< (const type&, const type&);

    template<typename Archive>
      void serialize (Archive& ar, unsigned int)
    {
      ar & _container;
    }

  private:
    std::vector<uint64_t> _container;
  };

  GSPC_DLLEXPORT type operator| (const type&, const type&);
  GSPC_DLLEXPORT type operator& (const type&, const type&);
  GSPC_DLLEXPORT type operator^ (const type&, const type&);

  GSPC_DLLEXPORT bool operator== (const type&, const type&);
  GSPC_DLLEXPORT bool operator< (const type&, const type&);

  GSPC_DLLEXPORT std::size_t hash_value (const type&);

  GSPC_DLLEXPORT std::ostream& operator<< (std::ostream&, const type&);

  GSPC_DLLEXPORT std::string to_hex (const type&);
  GSPC_DLLEXPORT type from_hex (const std::string&);
  GSPC_DLLEXPORT boost::optional<type> from_hex
    ( std::string::const_iterator& pos
    , const std::string::const_iterator& end
    );
}
