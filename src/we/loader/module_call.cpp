#include <we/loader/module_call.hpp>

#include <we/type/id.hpp>
#include <we/type/port.hpp>
#include <we/type/range.hpp>

#include <fvm-pc/pc.hpp>
// used
// fvmGetShmemPtr
// fvmGetShmemSize
// fvmGetGlobalData
// fvmPutGlobalData
// waitComm

//! \todo remove, needed to make a complete type
#include <we/type/net.hpp>

#include <boost/format.hpp>

#include <functional>
#include <unordered_map>

#include <string>

namespace we
{
  namespace
  {
    expr::eval::context input (we::type::activity_t const& activity)
    {
      expr::eval::context context;

      for ( std::pair< pnet::type::value::value_type
                     , we::port_id_type
                     > const& token_on_port
          : activity.input()
          )
      {
        context.bind_ref
          ( activity.transition().ports_input().at (token_on_port.second).name()
          , token_on_port.first
          );
      }

      return context;
    }

    unsigned long evaluate_size_or_die ( we::type::activity_t const& activity
                                       , std::string const& expression
                                       )
    {
      expr::eval::context context (input (activity));

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

    void transfer
      ( std::function<fvmCommHandle_t ( const fvmAllocHandle_t
                                      , const fvmOffset_t
                                      , const fvmSize_t
                                      , const fvmShmemOffset_t
                                      , const fvmAllocHandle_t
                                      )
                     > do_transfer
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

        waitComm
          ( do_transfer
            ( std::stoul (global.handle().name(), nullptr, 16)
            , global.offset()
            , local.size()
            , memory_buffer.at (local.buffer()).position() + local.offset()
            , 0
            )
          );
      }
    }
  }

  namespace loader
  {
    void module_call ( we::loader::loader& loader
                     , drts::worker::context* context
                     , we::type::activity_t& act
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
        char* const local_memory (static_cast<char*> (fvmGetShmemPtr()));

        unsigned long const size
          (evaluate_size_or_die (act, buffer_and_size.second));

        memory_buffer.emplace (buffer_and_size.first, buffer (position, size));
        pointers.emplace (buffer_and_size.first, local_memory + position);

        position += size;

        if (position > fvmGetShmemSize())
        {
          //! \todo specific exception
          throw std::runtime_error
            ( ( boost::format ("not enough local memory: %1% > %2%")
              % position
              % fvmGetShmemSize()
              ).str()
            );
        }
      }

      transfer
        (fvmGetGlobalData, memory_buffer, module_call.gets (input (act)));

      std::list<std::pair<local::range, global::range>> const
        puts_evaluated_before_call
          (module_call.puts_evaluated_before_call (input (act)));

      expr::eval::context out (input (act));

      loader[module_call.module()].call
        (module_call.function(), context, input (act), out, pointers);

      for ( std::pair<we::port_id_type, we::type::port_t> const& port_by_id
          : act.transition().ports_output()
          )
      {
        const we::port_id_type& port_id (port_by_id.first);
        const we::type::port_t& port (port_by_id.second);

        act.add_output (port_id, out.value (port.name()));
      }

      transfer (fvmPutGlobalData, memory_buffer, puts_evaluated_before_call);
      transfer ( fvmPutGlobalData
               , memory_buffer
               , module_call.puts_evaluated_after_call (out)
               );
    }
  }
}