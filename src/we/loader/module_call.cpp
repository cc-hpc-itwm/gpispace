#include <we/loader/module_call.hpp>

#include <we/type/id.hpp>
#include <we/type/port.hpp>
#include <we/type/range.hpp>
#include <we/expr/parse/parser.hpp>

#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/functor_visitor.hpp>

#include <vmem/intertwine_compat.hpp>
#include <vmem/operations.hpp>
#include <vmem/types.hpp>

#include <boost/format.hpp>

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <string>

namespace we
{
  namespace loader
  {
    namespace
    {
      unsigned long evaluate_size_or_die ( expr::eval::context context
                                         , std::string const& expression
                                         )
      {
        return boost::get<unsigned long>
          (expr::parse::parser (expression).eval_all (context));
      }
    }

    expr::eval::context module_call
      ( we::loader::loader& loader
      , intertwine::vmem::ipc_client* virtual_memory_api
      , boost::optional<intertwine::vmem::shared_cache_id_t> shared_cache
      , gspc::scoped_vmem_cache const* cache
      , drts::worker::context* context
      , expr::eval::context const& input
      , const we::type::module_call_t& module_call
      )
    {
      std::unordered_map<std::string, std::pair<std::size_t, bool>> memory_buffers;

      // Calculate actual sizes of memory buffers.
      for ( std::pair<std::string, we::type::memory_buffer_info> const& buffer
          : module_call.memory_buffers()
          )
      {
        unsigned long const size
          (evaluate_size_or_die (input, buffer.second.size()));

        memory_buffers.emplace
          ( std::piecewise_construct
          , std::forward_as_tuple (buffer.first)
          , std::forward_as_tuple (size, buffer.second.read_only())
          );
      }

      std::unordered_set<std::string> input_memory_buffers;
      std::unordered_map
        < std::string
        , boost::variant< intertwine::vmem::const_local_range_t
                        , intertwine::vmem::mutable_local_range_t
                        >
        > local_ranges;

      FHG_UTIL_FINALLY
        ( [&]
          {
            for (auto range : local_ranges)
            {
              fhg::util::visit<void>
                ( range.second
                , [&] (intertwine::vmem::const_local_range_t const& r)
                  {
                    try
                    {
                      boost::get<intertwine::vmem::void_t>
                        ( virtual_memory_api->execute_sync
                            ( intertwine::vmem::op::release_t
                                {boost::get<intertwine::vmem::const_local_range_t>(r)}
                            )
                        );
                    }
                    catch (boost::system::system_error const&)
                    {
                      //! \note Ignore: The virtual memory server somehow
                      //! disappeared. It will have killed the ranges with
                      //! itself.
                    }
                  }
                , [&] (intertwine::vmem::mutable_local_range_t const& r)
                  {
                    try
                    {
                      boost::get<intertwine::vmem::void_t>
                        ( virtual_memory_api->execute_sync
                            ( intertwine::vmem::op::release_t
                                {boost::get<intertwine::vmem::mutable_local_range_t>(r)}
                            )
                        );
                    }
                    catch (boost::system::system_error const&)
                    {
                      //! \note Ignore: The virtual memory server somehow
                      //! disappeared. It will have killed the ranges with
                      //! itself.
                    }
                  }
                );
            }
          }
        );

      std::map<std::string, void*> pointers;

      // Collect input buffers and fetch data.
      for (auto get : module_call.gets (input))
      {
        local::range const& local (get.first);
        global::range const& global (get.second);

        if (!memory_buffers.count (local.buffer()))
        {
          //! \todo specific exception
          throw std::runtime_error ("unknown memory buffer " + local.buffer());
        }

        auto const& buffer_info (memory_buffers.at (local.buffer()));
        if (local.offset() || local.size() != buffer_info.first)
        {
          //! \todo implement
          throw std::runtime_error
            ("unable to gather multiple gets into one buffer currently");
        }

        input_memory_buffers.emplace (local.buffer());

        if (buffer_info.second) //the buffer is read-only
        {
          if (!shared_cache)
          {
            throw std::runtime_error
              ("Attempting to get const data in unspecified or non-existing shared cache");
          }

          auto const local_range
            ( boost::get<intertwine::vmem::const_local_range_t>
                ( virtual_memory_api->execute_sync
                    ( intertwine::vmem::op::get_const_t
                        { fhg::vmem::intertwine_compat::global_range (global)
                        , *shared_cache
                        }
                    )
                )
            );
          pointers.emplace (local.buffer(), const_cast<void*>(local_range.pointer()));
          local_ranges.emplace (local.buffer(), local_range);
        }
        else
        {
          if (!cache)
          {
            throw std::runtime_error
              ("Attempting to get mutable data in unspecified or non-existing private cache");
          }

          auto const local_range
            ( boost::get<intertwine::vmem::mutable_local_range_t>
                ( virtual_memory_api->execute_sync
                    ( intertwine::vmem::op::get_mutable_t
                        { fhg::vmem::intertwine_compat::global_range (global)
                        , *cache
                        }
                    )
                )
            );
          pointers.emplace (local.buffer(), local_range.pointer());
          local_ranges.emplace (local.buffer(), local_range);
        }
      }

      // Allocate memory for pure output buffers.
      std::unordered_map<std::string, std::pair<std::size_t, bool>>
        pure_output_memory_buffers (memory_buffers);
      for (auto const& input : input_memory_buffers)
      {
        pure_output_memory_buffers.erase (input);
      }

      for (auto const& pure_output_buffer : pure_output_memory_buffers)
      {
        if (!cache)
        {
          throw std::runtime_error
            ("Attempting to allocate data in unspecified or non-existing private cache!");
        }

        auto const local_range
          ( boost::get<intertwine::vmem::mutable_local_range_t>
              ( virtual_memory_api->execute_sync
                  ( intertwine::vmem::op::allocate_t
                      { intertwine::vmem::size_t (pure_output_buffer.second.first)
                      , *cache
                      }
                  )
              )
          );
        pointers.emplace (pure_output_buffer.first, local_range.pointer());
        local_ranges.emplace (pure_output_buffer.first, local_range);
      }

      // Eval puts.
      std::list<std::pair<local::range, global::range>> const
        puts_evaluated_before_call
          (module_call.puts_evaluated_before_call (input));

      // Call.
      expr::eval::context out (input);

      {
        drts::worker::redirect_output const clog (context, fhg::log::TRACE, std::clog);
        drts::worker::redirect_output const cout (context, fhg::log::INFO, std::cout);
        drts::worker::redirect_output const cerr (context, fhg::log::WARN, std::cerr);

        loader[module_call.module()].call
          (module_call.function(), context, input, out, pointers);
      }

      // Put.
      for ( auto puts
          : { puts_evaluated_before_call
            , module_call.puts_evaluated_after_call (out)
            }
          )
      {
        for (auto put : puts)
        {
          local::range const& local (put.first);
          global::range const& global (put.second);

          if (!memory_buffers.count (local.buffer()))
          {
            //! \todo specific exception
            throw std::runtime_error ("unknown memory buffer " + local.buffer());
          }

          try
          {
            intertwine::vmem::mutable_local_range_t range
              ( boost::get<intertwine::vmem::mutable_local_range_t>
                  (local_ranges.at (local.buffer()))
              );

            range.range.offset += intertwine::vmem::offset_t (local.offset());
            range.range.size = intertwine::vmem::size_t (local.size());

            boost::get<intertwine::vmem::void_t>
              ( virtual_memory_api->execute_sync
                  ( intertwine::vmem::op::put_t
                      {range, fhg::vmem::intertwine_compat::global_range (global)}
                  )
              );
          }
          catch (boost::bad_get const&)
          {
            throw std::runtime_error
              ("invalid local range type appearing in a put operation");
          }
        }
      }

      return out;
    }
  }
}
