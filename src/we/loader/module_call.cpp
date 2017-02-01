#include <we/loader/module_call.hpp>

#include <we/type/id.hpp>
#include <we/type/port.hpp>
#include <we/type/range.hpp>
#include <we/expr/parse/parser.hpp>

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
      , gspc::scoped_vmem_cache& vmem_cache
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
            (vmem_cache, shared_memory_offset)
        , size
        );
    }

    void get_global_data
      ( gpi::pc::client::api_t /*const*/& virtual_memory_api
      , gspc::scoped_vmem_cache& vmem_cache
      , const fvmAllocHandle_t global_memory_handle
      , const fvmOffset_t global_memory_offset
      , const fvmSize_t size
      , const fvmShmemOffset_t shared_memory_offset
      )
    {
      virtual_memory_api.memcpy_and_wait
        ( gpi::pc::type::memory_location_t
            (vmem_cache, shared_memory_offset)
        , gpi::pc::type::memory_location_t
            (global_memory_handle, global_memory_offset)
        , size
        );
    }

    void transfer
      ( std::function<void
                      ( gpi::pc::client::api_t /*const*/&
                      , gspc::scoped_vmem_cache /*const*/&
                      , const fvmAllocHandle_t
                      , const fvmOffset_t
                      , const fvmSize_t
                      , const fvmShmemOffset_t
                      )> do_transfer
      , gpi::pc::client::api_t /*const*/* virtual_memory_api
      , gspc::scoped_vmem_cache* vmem_cache
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
          , *vmem_cache
          , std::stoul (global.handle().name(), nullptr, 16)
          , global.offset()
          , local.size()
          , memory_buffer.at (local.buffer()).position() + local.offset()
          );
      }
    }
  }

  namespace loader
  {
    expr::eval::context module_call
      ( we::loader::loader& loader
      , gpi::pc::client::api_t /*const*/* virtual_memory_api
      , gspc::scoped_vmem_cache* vmem_cache
      , drts::worker::context* context
      , expr::eval::context const& input
      , const we::type::module_call_t& module_call
      )
    {
      unsigned long position (0);

      std::map<std::string, void*> pointers;
      std::unordered_map<std::string, buffer> memory_buffer;

      for ( std::pair<std::string, std::string> const& buffer_and_size
          : module_call.memory_buffers()
          )
      {
        if (!virtual_memory_api || !vmem_cache)
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

        char* const local_memory
          (static_cast<char*> (virtual_memory_api->ptr (*vmem_cache)));

        unsigned long const size
          (evaluate_size_or_die (input, buffer_and_size.second));

        memory_buffer.emplace (buffer_and_size.first, buffer (position, size));
        pointers.emplace (buffer_and_size.first, local_memory + position);

        position += size;

        if (position > vmem_cache->size())
        {
          //! \todo specific exception
          throw std::runtime_error
            ( ( boost::format ("not enough local memory: %1% > %2%")
              % position
              % vmem_cache->size()
              ).str()
            );
        }
      }

      transfer ( get_global_data, virtual_memory_api, vmem_cache
               , memory_buffer, module_call.gets (input)
               );

      std::list<std::pair<local::range, global::range>> const
        puts_evaluated_before_call
          (module_call.puts_evaluated_before_call (input));

      expr::eval::context out (input);

      {
        drts::worker::redirect_output const clog (context, fhg::log::TRACE, std::clog);
        drts::worker::redirect_output const cout (context, fhg::log::INFO, std::cout);
        drts::worker::redirect_output const cerr (context, fhg::log::WARN, std::cerr);

        loader[module_call.module()].call
          (module_call.function(), context, input, out, pointers);
      }

      transfer ( put_global_data, virtual_memory_api, vmem_cache
               , memory_buffer, puts_evaluated_before_call
               );
      transfer ( put_global_data, virtual_memory_api, vmem_cache
               , memory_buffer, module_call.puts_evaluated_after_call (out)
               );

      return out;
    }
  }
}
