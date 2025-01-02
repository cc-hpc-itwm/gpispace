// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/eval/context.hpp>
#include <we/loader/loader.hpp>
#include <we/type/ModuleCall.hpp>

namespace iml
{
  class Client;
  class SharedMemoryAllocation;
}

namespace we
{
  namespace loader
  {
    expr::eval::context module_call
      ( we::loader::loader& loader
      , iml::Client /*const*/*
      , iml::SharedMemoryAllocation /*const*/*
      , drts::worker::context* context
      , expr::eval::context const& input
      , we::type::ModuleCall const& module_call
      );
  }
}
