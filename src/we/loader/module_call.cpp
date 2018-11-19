#include <we/loader/module_call.hpp>

#include <we/type/id.hpp>
#include <we/type/port.hpp>
#include <we/type/range.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/type/value.hpp>
#include <we/type/value/serialize.hpp>

#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

#include <boost/format.hpp>

#include <functional>
#include <unordered_map>

#include <string>

namespace we
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

    std::string evaluate_dataid_or_die ( expr::eval::context context
                                       , std::string const& expression
                                       )
    {
      return //pnet::type::value::to_string
          std::to_string(boost::get<unsigned long>(expr::parse::parser (expression).eval_all (context)));
    }

    class buffer
	{
	public:
    	buffer (unsigned long position, unsigned long size)
	: _size (size)
	, _position (position)
	{}

    	unsigned long position() const
    	{
    		return _position;
    	}
    	unsigned long size() const
    	{
    		return _size;
    	}

	private:
    	unsigned long _size;
    	unsigned long _position;
	};

    typedef unsigned long fvmAllocHandle_t;
    typedef unsigned long fvmSize_t;
    typedef unsigned long fvmOffset_t;
    typedef unsigned long fvmShmemOffset_t;

    void put_global_data
      ( gpi::pc::client::api_t /*const*/& virtual_memory_api
      , gspc::scoped_allocation /*const*/& shared_memory
      , const fvmAllocHandle_t global_memory_handle
      , const fvmOffset_t global_memory_offset
      , const fvmSize_t size
      , const fvmShmemOffset_t shared_memory_offset
      )
    {
      virtual_memory_api.memcpy_and_wait
        ( gpi::pc::type::memory_location_t
            (global_memory_handle, global_memory_offset)
        , gpi::pc::type::memory_location_t
            (shared_memory, shared_memory_offset)
        , size
        );
    }

    void get_global_data
      ( gpi::pc::client::api_t /*const*/& virtual_memory_api
      , gspc::scoped_allocation /*const*/& shared_memory
      , const fvmAllocHandle_t global_memory_handle
      , const fvmOffset_t global_memory_offset
      , const fvmSize_t size
      , const fvmShmemOffset_t shared_memory_offset
      )
    {
      virtual_memory_api.memcpy_and_wait
        ( gpi::pc::type::memory_location_t
            (shared_memory, shared_memory_offset)
        , gpi::pc::type::memory_location_t
            (global_memory_handle, global_memory_offset)
        , size
        );
    }

    void transfer
      ( std::function<void
                      ( gpi::pc::client::api_t /*const*/&
                      , gspc::scoped_allocation /*const*/&
                      , const fvmAllocHandle_t
                      , const fvmOffset_t
                      , const fvmSize_t
                      , const fvmShmemOffset_t
                      )> do_transfer
      , gpi::pc::client::api_t /*const*/* virtual_memory_api
      , gspc::scoped_allocation /*const*/* shared_memory
      , std::unordered_map<std::string, buffer> const& memory_buffer
      , std::unordered_map<std::string, std::string> const& cached_dataids
      , std::list<std::pair<local::range, global::range>> const& transfers
      )
    {
      for (std::pair<local::range, global::range> const& transfer : transfers)
      {
        local::range const& local (transfer.first);
        global::range const& global (transfer.second);

        fhg_assert (local.size() == global.size());

        if (!memory_buffer.count (local.buffer()))
        {
          //! \todo specific exception
          throw std::runtime_error ("unknown memory buffer " + local.buffer());
        }

        if ( local.offset() + local.size()
           > memory_buffer.at (local.buffer()).size()
           )
        {
          //! \todo specific exception
          throw std::runtime_error ("local range too large");
        }

        //! \todo check global range, needs knowledge about global memory

        if (!cached_dataids.count(local.buffer())) // only perform transfer if the buffer is not already cached
        {
          do_transfer
            ( *virtual_memory_api
            , *shared_memory
            , std::stoul (global.handle().name(), nullptr, 16)
            , global.offset()
            , local.size()
            , memory_buffer.at (local.buffer()).position() + local.offset()
            );
        }
      }
    }
  }

  namespace loader
  {
    expr::eval::context module_call
      ( we::loader::loader& loader
      , gpi::pc::client::api_t /*const*/* virtual_memory_api
      , gspc::scoped_allocation /*const*/* shared_memory
      , drts::cache::cache_manager* cache
      , drts::worker::context* context
      , expr::eval::context const& input
      , const we::type::module_call_t& module_call
      , std::function < void ( sdpa::daemon::NotificationEvent::type_t
                             , sdpa::daemon::NotificationEvent::state_t
                             )
                      > const& emit_gantt
      )
    {
      if (module_call.memory_buffers().size()>0 && (!virtual_memory_api || !shared_memory))
      {
        throw std::logic_error
        ( ( boost::format
            ( "module call '%1%::%2%' with %3% memory transfers scheduled "
                "to worker '%4%' that is unable to manage memory"
            )
        % module_call.module()
        % module_call.function()
        % module_call.memory_buffers().size()
        % context->worker_name()
        ).str()
        );
      }

      unsigned long position(cache?cache->size():0);

      std::map<std::string, void*> pointers;
      std::unordered_map<std::string, buffer> memory_buffer;

      std::unordered_map<std::string, unsigned long> dataid_and_sizes;
      std::unordered_map<std::string, std::string> buffer_and_dataids;  // save <buffer name, dataid> pairs to avoid re-evaluating dataids
      for ( std::pair<std::string, std::string> const& buffer_and_size
          : module_call.memory_buffers()
      )
      {
        char* const local_memory
          (static_cast<char*> (virtual_memory_api->ptr (*shared_memory)));

        unsigned long const size
          (evaluate_size_or_die(input, buffer_and_size.second));

        if (module_call.memory_buffer_dataids().count(buffer_and_size.first) > 0) // the buffer is a cached one
        {
          auto dataid = evaluate_dataid_or_die(input, module_call.memory_buffer_dataids().at(buffer_and_size.first));

          dataid_and_sizes.emplace(dataid, size);
          buffer_and_dataids.emplace(buffer_and_size.first, dataid);

        } else {
          // no dataid, allocate buffer in the remaining local memory and advance the currently available position
          memory_buffer.emplace (buffer_and_size.first, buffer (position, size));
          pointers.emplace (buffer_and_size.first, local_memory + position);

          position += size;

          if (position > shared_memory->size())
          {
            //! \todo specific exception
            throw std::runtime_error
            ( ( boost::format ("not enough local memory: %1% > %2%")
            % position
            % shared_memory->size()
            ).str()
            );
          }
        }
      }


      std::unordered_map<std::string, std::string> cached_dataids;
      if (dataid_and_sizes.size() > 0)
      {
        if (!cache)
        {
          throw std::runtime_error("the workflow contains cached buffers, but no cache size defined at run-time");
        }

        auto already_cached_dataids = cache->add_chunk_list_to_cache(dataid_and_sizes);

        // now we are sure all cached buffers have associated offsets
        // we can add them to the list of local memory pointers
        for ( std::pair<std::string, std::string> const& buffer_and_size
            : module_call.memory_buffers()
            )
        {
          char* const local_memory
          (static_cast<char*> (virtual_memory_api->ptr (*shared_memory)));

          if (buffer_and_dataids.count(buffer_and_size.first) > 0)  // this is a cached buffer
          {
            auto dataid = buffer_and_dataids.at(buffer_and_size.first);

            if (already_cached_dataids.count(dataid) > 0)
            {
              cached_dataids.emplace(buffer_and_size.first, dataid);
            }
            position = cache->offset(dataid);
            memory_buffer.emplace(buffer_and_size.first, buffer(position, dataid_and_sizes.at(dataid)));
            pointers.emplace(buffer_and_size.first, local_memory + position);

          }
        }
      }

      using NotificationEvent = sdpa::daemon::NotificationEvent;

      emit_gantt ( NotificationEvent::type_t::vmem_get
                 , NotificationEvent::STATE_STARTED
                 );

      transfer ( get_global_data, virtual_memory_api, shared_memory
               , memory_buffer, cached_dataids, module_call.gets (input)
               );

      emit_gantt ( NotificationEvent::type_t::vmem_get
                 , NotificationEvent::STATE_FINISHED
                 );

      std::list<std::pair<local::range, global::range>> const
        puts_evaluated_before_call
          (module_call.puts_evaluated_before_call (input));

      expr::eval::context out (input);

      emit_gantt ( NotificationEvent::type_t::module_call
                 , NotificationEvent::STATE_STARTED
                 );

      try
      {
        drts::worker::redirect_output const clog (context, fhg::log::TRACE, std::clog);
        drts::worker::redirect_output const cout (context, fhg::log::INFO, std::cout);
        drts::worker::redirect_output const cerr (context, fhg::log::WARN, std::cerr);

        loader[module_call.module()].call
          (module_call.function(), context, input, out, pointers);
      }
      catch (...)
      {
        //! \todo HACK HACK HACK: never emits canceled or
        //! canceled_due_to_worker_shutdown!
        emit_gantt ( NotificationEvent::type_t::module_call
                   , NotificationEvent::STATE_FAILED
                   );
        throw;
      }

      emit_gantt ( NotificationEvent::type_t::module_call
                 , NotificationEvent::STATE_FINISHED
                 );

      emit_gantt ( NotificationEvent::type_t::vmem_put
                 , NotificationEvent::STATE_STARTED
                 );

      transfer ( put_global_data, virtual_memory_api, shared_memory
               , memory_buffer, cached_dataids, puts_evaluated_before_call
               );
      transfer ( put_global_data, virtual_memory_api, shared_memory
               , memory_buffer, cached_dataids, module_call.puts_evaluated_after_call (out)
               );

      emit_gantt ( NotificationEvent::type_t::vmem_put
                 , NotificationEvent::STATE_FINISHED
                 );

      return out;
    }
  }
}
