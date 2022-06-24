// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <set>
#include <string>

namespace sdpa
{
  // type, name
  class Capability
  {
  public:
    Capability() = default;
    explicit Capability (std::string const&);

    std::string name() const;

    template <class Archive>
      void serialize (Archive&, unsigned int);

    bool operator< (Capability const&) const;
    bool operator== (Capability const&) const;

  private:
    std::string name_;
    std::string uuid_;
  };

  using Capabilities = std::set<Capability>;
}

#include "capability.ipp"
