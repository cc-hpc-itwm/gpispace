// mirko.rahn@itwm.fraunhofer.de

#include <we/type/module_call.fwd.hpp>
#include <we/type/module_call.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/type/value.hpp>

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
    }

    std::unordered_map<std::string, unsigned long>
      module_call_t::memory_buffer_sizes
        (expr::eval::context const& input) const
    {
      std::unordered_map<std::string, unsigned long> sizes;

      for ( std::pair<std::string, std::string> const& name_and_expression
          : memory_buffers()
          )
      {
        expr::eval::context context (input);

        sizes.emplace ( name_and_expression.first
                      , boost::get<unsigned long>
                          (expr::parse::parser (name_and_expression.second)
                            .eval_all (context)
                          )
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

        zip ( evaluate<local::range> (context, mg.local())
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

          zip ( evaluate<local::range> (context, mp.local())
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

          zip ( evaluate<local::range> (context, mp.local())
              , evaluate<global::range> (context, mp.global())
              , puts
              );
        }
      }

      return puts;
    }
  }
}
