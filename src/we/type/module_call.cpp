// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <we/type/module_call.fwd.hpp>
#include <we/type/module_call.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/type/value.hpp>

#include <algorithm>
#include <stdexcept>

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

    bool module_call_t::require_function_unloads_without_rest() const
    {
      return _require_function_unloads_without_rest;
    }
    bool module_call_t::require_module_unloads_without_rest() const
    {
      return _require_module_unloads_without_rest;
    }

    std::unordered_map<std::string, unsigned long>
      module_call_t::memory_buffer_sizes
        (expr::eval::context const& input) const
    {
      std::unordered_map<std::string, unsigned long> sizes;

      for (auto const& name_and_buffer_info : memory_buffers())
      {
        sizes.emplace
          ( name_and_buffer_info.first
          , name_and_buffer_info.second.size (input)
          );
      }

      return sizes;
    }

    std::list<std::pair<local::range, global::range>>
      module_call_t::gets (expr::eval::context const& input) const
    {
      std::list<std::pair<local::range, global::range>> gets;

      for (type::memory_transfer const& mg : memory_gets())
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
      module_call_t::puts_evaluated_before_call
        (expr::eval::context const& output) const
    {
      std::list<std::pair<local::range, global::range>> puts;

      for (type::memory_transfer const& mp : memory_puts())
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
      module_call_t::puts_evaluated_after_call
        (expr::eval::context const& output) const
    {
      std::list<std::pair<local::range, global::range>> puts;

      for (type::memory_transfer const& mp : memory_puts())
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
  }
}
