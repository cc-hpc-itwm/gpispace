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

#include <algorithm>
#include <string>
#include <set>

namespace sdpa
{
  // type, name
  class Capability
  {
  public:
    Capability() = default;
    explicit Capability(const std::string& name,
                        const std::string& owner);

    std::string name() const { return name_;}

    std::string owner() const { return owner_; }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
      ar & name_;
      ar & owner_;
      ar & uuid_;
    }

    bool operator<(const Capability& b) const
    {
      return uuid_ < b.uuid_;
    }

    bool operator==(const Capability& b) const
    {
      return uuid_ == b.uuid_;
    }

  private:
    std::string name_;
    std::string owner_;
    std::string uuid_;
  };

  typedef Capability capability_t;

  typedef std::set<capability_t> capabilities_set_t;
}
