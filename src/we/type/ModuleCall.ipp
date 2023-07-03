// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace we
{
  namespace type
  {
    template<typename Archive>
      void ModuleCall::serialize (Archive& ar, unsigned int)
    {
      ar & module_;
      ar & function_;
      ar & _memory_buffers;
      ar & _memory_gets;
      ar & _memory_puts;
      ar & _require_function_unloads_without_rest;
      ar & _require_module_unloads_without_rest;
    }
  }
}
