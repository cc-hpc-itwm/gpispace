// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/ModuleCall.fwd.hpp>

#include <we/expr/eval/context.hpp>
#include <we/expr/type/Context.hpp>
#include <we/type/MemoryBufferInfo.hpp>
#include <we/type/memory_transfer.hpp>
#include <we/type/range.hpp>

#include <iosfwd>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>

namespace we { namespace type {
  struct ModuleCall
  {
    ModuleCall () = default;

    ModuleCall
      ( std::string const& module
      , std::string const& function
      , std::unordered_map<std::string, MemoryBufferInfo>&& memory_buffers
      , std::list<memory_transfer>&& memory_gets
      , std::list<memory_transfer>&& memory_puts
      , bool require_function_unloads_without_rest
      , bool require_module_unloads_without_rest
      );

    std::string const& module() const;
    std::string const& function() const;

    bool require_function_unloads_without_rest() const;
    bool require_module_unloads_without_rest() const;

    std::unordered_map<std::string, MemoryBufferInfo> const&
      memory_buffers() const;

    std::list<std::pair<local::range, global::range>>
      puts_evaluated_before_call (expr::eval::context const&) const;
    std::list<std::pair<local::range, global::range>>
      puts_evaluated_after_call (expr::eval::context const&) const;
    std::list<std::pair<local::range, global::range>>
      gets (expr::eval::context const&) const;

    unsigned long memory_buffer_size_total (expr::eval::context const&) const;

    void assert_correct_expression_types
      ( expr::type::Context const& before_eval
      , expr::type::Context const& after_eval
      ) const;

    private:
    std::string module_;
    std::string function_;
    std::unordered_map<std::string, MemoryBufferInfo> _memory_buffers;
    std::list<memory_transfer> _memory_gets;
    std::list<memory_transfer> _memory_puts;
    bool _require_function_unloads_without_rest;
    bool _require_module_unloads_without_rest;

    friend class ::boost::serialization::access;
    template<typename Archive> void serialize (Archive&, unsigned int);
  };

  std::ostream& operator<< (std::ostream& os, ModuleCall const& m);
}}

#include <we/type/ModuleCall.ipp>
