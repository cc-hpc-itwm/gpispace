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

#include <we/type/ModuleCall.fwd.hpp>
#include <we/type/ModuleCall.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/type/value.hpp>

#include <util-generic/nest_exceptions.hpp>

#include <boost/format.hpp>

#include <algorithm>
#include <exception>
#include <iterator>
#include <list>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace we
{
  namespace type
  {
    namespace
    {
      template<typename Range>
        std::list<Range> evaluate ( expr::eval::context& context
                                  , std::string const& expression
                                  )
      {
        std::list<pnet::type::value::value_type> const values
          ( boost::get<std::list<pnet::type::value::value_type>>
            (expr::parse::parser (expression).eval_all (context))
          );

        return {values.begin(), values.end()};
      }

      void zip ( std::list<local::range>&& local_ranges
               , std::list<global::range>&& global_ranges
               , std::list<std::pair<local::range, global::range>>& zipped
               )
      {
        std::list<local::range>::iterator local (local_ranges.begin());
        std::list<local::range>::iterator const local_end (local_ranges.end());
        std::list<global::range>::iterator global (global_ranges.begin());
        std::list<global::range>::iterator const global_end (global_ranges.end());

        while (local != local_end && global != global_end)
        {
          unsigned long const min_size
            (std::min (local->size(), global->size()));

          zipped.emplace_back
            ( std::make_pair ( local::range (*local, min_size)
                             , global::range (*global, min_size)
                             )
            );

          local->shrink (min_size);
          global->shrink (min_size);

          if (local->size() == 0)
          {
            ++local;
          }
          if (global->size() == 0)
          {
            ++global;
          }
        }

        if (local != local_end || global != global_end)
        {
          //! \todo specific exception
          throw std::runtime_error ("sum of sizes of ranges differ");
        }
      }

      std::list<local::range>
        evaluate_and_trigger_error_if_empty_ranges_are_not_allowed
          ( expr::eval::context& context
          , type::memory_transfer const& transfer
          )
      {
        auto local_ranges
          (evaluate<local::range> (context, transfer.local()));

        if (!transfer.allow_empty_ranges())
        {
          if ( std::any_of ( local_ranges.begin()
                           , local_ranges.end()
                           , [] (local::range const& range)
                             {
                               return range.size() == 0;
                             }
                           )
             )
          {
            throw std::runtime_error
              ( "Attempting to transfer empty ranges! If this behavior is "
                "wanted, then please set the attribute \"allow-empty-ranges\" "
                "for memory transfers on true in the Petri net!"
              );
           }
        }

        return local_ranges;
      }
    }

    ModuleCall::ModuleCall
      ( std::string const& module
      , std::string const& function
      , std::unordered_map<std::string, MemoryBufferInfo>&& memory_buffers
      , std::list<memory_transfer>&& memory_gets
      , std::list<memory_transfer>&& memory_puts
      , bool require_function_unloads_without_rest
      , bool require_module_unloads_without_rest
      )
      : module_(module)
      , function_(function)
      , _memory_buffers (memory_buffers)
      , _memory_gets (memory_gets)
      , _memory_puts (memory_puts)
      , _require_function_unloads_without_rest
          (require_function_unloads_without_rest)
      , _require_module_unloads_without_rest
          (require_module_unloads_without_rest)
    {}

    std::string const& ModuleCall::module() const
    {
      return module_;
    }

    std::string const& ModuleCall::function() const
    {
      return function_;
    }

    std::unordered_map<std::string, MemoryBufferInfo> const&
      ModuleCall::memory_buffers() const
    {
      return _memory_buffers;
    }

    bool ModuleCall::require_function_unloads_without_rest() const
    {
      return _require_function_unloads_without_rest;
    }
    bool ModuleCall::require_module_unloads_without_rest() const
    {
      return _require_module_unloads_without_rest;
    }

    unsigned long ModuleCall::memory_buffer_size_total
      (expr::eval::context const& context) const
    {
      return std::accumulate
        ( std::begin (_memory_buffers), std::end (_memory_buffers)
        , 0ul
        , [&] (auto sum, auto const& buffer)
          {
            return sum + buffer.second.size (context);
          }
        );
    }

    void ModuleCall::assert_correct_expression_types
      ( expr::type::Context const& context_before_eval
      , expr::type::Context const& context_after_eval
      ) const
    try
    {
      for (auto const& buffer : _memory_buffers)
      {
        fhg::util::nest_exceptions<std::runtime_error>
          ( [&]
            {
              buffer.second.assert_correct_expression_types
                (context_before_eval);
            }
          , str ( boost::format ("In the <buffer> with name '%1%'")
                % buffer.first
                )
          );
      }

      for (auto const& memory_get : _memory_gets)
      {
        fhg::util::nest_exceptions<std::runtime_error>
          ( [&]
            {
              memory_get.assert_correct_expression_types
                (context_before_eval);
            }
          , "In <memory-get>"
          );
      }

      for (auto const& memory_put : _memory_puts)
      {
        fhg::util::nest_exceptions<std::runtime_error>
          ( [&]
            {
              memory_put.assert_correct_expression_types
                ( memory_put.not_modified_in_module_call().get_value_or (false)
                ? context_before_eval
                : context_after_eval
                );
            }
          , str
            ( boost::format ("In <memory-put>, evaluated %1% execution")
            % ( memory_put.not_modified_in_module_call().get_value_or (false)
              ? "before"
              : "after"
              )
            )
          );
      }
    }
    catch (...)
    {
      std::throw_with_nested
        ( std::runtime_error
          (str ( boost::format ("In the function '%1%' of module '%2%'")
               % function_
               % module_
               )
          )
        );
    }

    std::list<std::pair<local::range, global::range>>
      ModuleCall::gets (expr::eval::context const& input) const
    {
      std::list<std::pair<local::range, global::range>> gets;

      for (type::memory_transfer const& mg : _memory_gets)
      {
        expr::eval::context context (input);

        zip ( evaluate_and_trigger_error_if_empty_ranges_are_not_allowed
                (context, mg)
            , evaluate<global::range> (context, mg.global())
            , gets
            );
      }

      return gets;
    }

    std::list<std::pair<local::range, global::range>>
      ModuleCall::puts_evaluated_before_call
        (expr::eval::context const& output) const
    {
      std::list<std::pair<local::range, global::range>> puts;

      for (type::memory_transfer const& mp : _memory_puts)
      {
        if (mp.not_modified_in_module_call().get_value_or (false))
        {
          expr::eval::context context (output);

          zip ( evaluate_and_trigger_error_if_empty_ranges_are_not_allowed
                  (context, mp)
              , evaluate<global::range> (context, mp.global())
              , puts
              );
        }
      }

      return puts;
    }

    std::list<std::pair<local::range, global::range>>
      ModuleCall::puts_evaluated_after_call
        (expr::eval::context const& output) const
    {
      std::list<std::pair<local::range, global::range>> puts;

      for (type::memory_transfer const& mp : _memory_puts)
      {
        if (!mp.not_modified_in_module_call().get_value_or (false))
        {
          expr::eval::context context (output);

          zip ( evaluate_and_trigger_error_if_empty_ranges_are_not_allowed
                  (context, mp)
              , evaluate<global::range> (context, mp.global())
              , puts
              );
        }
      }

      return puts;
    }

    std::ostream& operator<< (std::ostream& os, ModuleCall const& m)
    {
      return os << m.module() << "." << m.function();
    }
  }
}
