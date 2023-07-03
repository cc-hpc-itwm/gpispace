// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
