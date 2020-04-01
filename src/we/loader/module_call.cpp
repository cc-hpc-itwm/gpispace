#include <we/loader/module_call.hpp>

#include <we/loader/exceptions.hpp>

#include <we/type/id.hpp>
#include <we/type/port.hpp>
#include <we/type/range.hpp>
#include <we/expr/parse/parser.hpp>

#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

#include <boost/align/align.hpp>
#include <boost/format.hpp>

#include <functional>
#include <unordered_map>

#include <string>

namespace we
{
  namespace
  {
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
          throw std::runtime_error ("local range to large");
        }

        //! \todo check global range, needs knowledge about global memory

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

  namespace
  {
    template<typename T>
      bool align (std::size_t alignment, std::size_t size, T*& ptr, std::size_t& space)
    {
      void* ptr_void (ptr);
      auto const result (boost::alignment::align (alignment, size, ptr_void, space));
      ptr = static_cast<T*> (ptr_void);
      return result != nullptr;
    }
  }

  namespace loader
  {
    expr::eval::context module_call
      ( we::loader::loader& loader
      , gpi::pc::client::api_t /*const*/* virtual_memory_api
      , gspc::scoped_allocation /*const*/* shared_memory
      , drts::worker::context* context
      , expr::eval::context const& input
      , const we::type::module_call_t& module_call
      )
    {
      std::map<std::string, void*> pointers;
      std::unordered_map<std::string, buffer> memory_buffer;

      if (!module_call.memory_buffers().empty())
      {
        if (!virtual_memory_api)
        {
          throw std::logic_error
            ( ( boost::format
                  ( "module call '%1%::%2%' with %3% memory transfers "
                    "scheduled to worker '%4%' that is unable to manage "
                    "memory: no handler for the virtual memory was provided."
                  )
              % module_call.module()
              % module_call.function()
              % module_call.memory_buffers().size()
              % context->worker_name()
              ).str()
            );
          }

        if (!shared_memory)
        {
          throw std::logic_error
            ( ( boost::format
                  ( "module call '%1%::%2%' with %3% memory transfers "
                    "scheduled to worker '%4%' that is unable to manage "
                    "memory: no local shared memory was allocated."
                  )
              % module_call.module()
              % module_call.function()
              % module_call.memory_buffers().size()
              % context->worker_name()
              ).str()
            );
        }

        std::size_t const shared_memory_size (shared_memory->size());
        auto const total_size_required
          (module_call.memory_buffer_size_total (input));
        if (total_size_required > shared_memory_size)
        {
          throw std::runtime_error
            ( ( boost::format
                  ("not enough local memory allocated: %1% bytes required, "
                   "only %2% bytes allocated"
                  )
              % total_size_required
              % shared_memory_size
              ).str()
            );
         }

        char* const local_memory
          ((static_cast<char*> (virtual_memory_api->ptr (*shared_memory))));
        char* buffer_ptr (local_memory);
        std::size_t space (shared_memory_size);

        for (auto const& buffer_and_info : module_call.memory_buffers())
        {
          unsigned long const size (buffer_and_info.second.size (input));
          unsigned long const alignment
            (buffer_and_info.second.alignment (input));

          if (!align (alignment, size, buffer_ptr, space))
          {
            throw std::runtime_error
              ( ( boost::format
                    ("Not enough local memory: %1% > %2%. "
                     "Please take into account also the buffer alignments "
                     "when allocating local shared memory!"
                    )
                % (buffer_ptr - local_memory + size)
                % shared_memory_size
                ).str()
	      );
       	  }

          memory_buffer.emplace
            ( std::piecewise_construct
            , std::forward_as_tuple (buffer_and_info.first)
            , std::forward_as_tuple
                (buffer_ptr - local_memory, size)
            );
          pointers.emplace (buffer_and_info.first, buffer_ptr);

          buffer_ptr = buffer_ptr + size;
          space -= size;
        }
      }

      transfer ( get_global_data, virtual_memory_api, shared_memory
               , memory_buffer, module_call.gets (input)
               );

      std::list<std::pair<local::range, global::range>> const
        puts_evaluated_before_call
          (module_call.puts_evaluated_before_call (input));

      expr::eval::context out (input);

      {
        drts::worker::redirect_output const clog
          (context, fhg::logging::legacy::category_level_trace, std::clog);
        drts::worker::redirect_output const cout
          (context, fhg::logging::legacy::category_level_info, std::cout);
        drts::worker::redirect_output const cerr
          (context, fhg::logging::legacy::category_level_warn, std::cerr);

        auto const& module (loader[module_call.module()]);

        auto const before (fhg::util::currently_loaded_libraries());
        module.call
          (module_call.function(), context, input, out, pointers);
        auto const after (fhg::util::currently_loaded_libraries());

        if (before != after)
        {
          throw function_does_not_unload
            (module_call.module(), module_call.function(), before, after);
        }
      }

      transfer ( put_global_data, virtual_memory_api, shared_memory
               , memory_buffer, puts_evaluated_before_call
               );
      transfer ( put_global_data, virtual_memory_api, shared_memory
               , memory_buffer, module_call.puts_evaluated_after_call (out)
               );

      return out;
    }
  }
}
