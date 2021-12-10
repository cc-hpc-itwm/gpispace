// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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
