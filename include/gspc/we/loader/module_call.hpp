// Copyright (C) 2010,2012-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/expr/eval/context.hpp>
#include <gspc/we/loader/loader.hpp>
#include <gspc/we/type/ModuleCall.hpp>

namespace gspc::iml
{
  class Client;
  class SharedMemoryAllocation;
}


  namespace gspc::we::loader
  {
    we::expr::eval::context module_call
      ( we::loader::loader& loader
      , iml::Client /*const*/*
      , iml::SharedMemoryAllocation /*const*/*
      , drts::worker::context* context
      , we::expr::eval::context const& input
      , we::type::ModuleCall const& module_call
      );
  }
