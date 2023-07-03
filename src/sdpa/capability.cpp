// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/capability.hpp>
#include <sdpa/id_generator.hpp>

namespace sdpa
{
  namespace
  {
    id_generator& GLOBAL_id_generator_cap()
    {
      static id_generator g ("cap");
      return g;
    }
  }

  Capability::Capability (std::string const& name)
    : name_ (name)
    , uuid_ (GLOBAL_id_generator_cap().next())
    {}

  std::string Capability::name() const
  {
    return name_;
  }

  bool Capability::operator< (Capability const& b) const
  {
    return uuid_ < b.uuid_;
  }

  bool Capability::operator== (Capability const& b) const
  {
    return uuid_ == b.uuid_;
  }
}
