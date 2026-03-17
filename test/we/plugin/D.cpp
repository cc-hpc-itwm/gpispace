// Copyright (C) 2019-2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/plugin/Base.hpp>

#include <gspc/util/unreachable.hpp>

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
