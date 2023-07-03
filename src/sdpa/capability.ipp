// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace sdpa
{
  template<class Archive>
    void Capability::serialize (Archive& ar, unsigned int)
  {
    ar & name_;
    ar & uuid_;
  }
}
