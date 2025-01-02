// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/plugin/Base.hpp>

#include <util-generic/unreachable.hpp>

#include <stdexcept>

struct D : public gspc::we::plugin::Base
{
  [[noreturn]]
    D (::gspc::we::plugin::Context const&, ::gspc::we::plugin::PutToken)
  {
    throw std::runtime_error ("D::D()");
  }

  void before_eval (::gspc::we::plugin::Context const&) override
  {
    FHG_UTIL_UNREACHABLE();
  }
  void after_eval (::gspc::we::plugin::Context const&) override
  {
    FHG_UTIL_UNREACHABLE();
  }
};

GSPC_WE_PLUGIN_CREATE (D)
