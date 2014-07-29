#include <we/loader/module_call.hpp>

#include <we/type/id.hpp>
#include <we/type/port.hpp>
#include <we/expr/parse/parser.hpp>
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

    template<typename Range>
      std::list<Range> evaluate ( expr::eval::context& context
                                , std::string const& expression
                                )
    {
      std::list<pnet::type::value::value_type> const values
        ( boost::get<std::list<pnet::type::value::value_type>>
          (expr::parse::parser (expression).eval_all (context))
        );

      std::list<Range> ranges;

      for (pnet::type::value::value_type const& value : values)
      {
        ranges.emplace_back (value);
      }

      return ranges;
    }

    std::list<std::pair<local::range, global::range>>
      zip ( std::list<local::range>&& local_ranges
          , std::list<global::range>&& global_ranges
          )
    {
      std::list<std::pair<local::range, global::range>> zipped;

      std::list<local::range>::iterator local (local_ranges.begin());
      std::list<local::range>::iterator const local_end (local_ranges.end());
      std::list<global::range>::iterator global (global_ranges.begin());
      std::list<global::range>::iterator const global_end (global_ranges.end());

      while (local != local_end && global != global_end)
      {
        unsigned long const min_size (std::min (local->size(), global->size()));

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

      return zipped;
    }

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

    std::list<std::pair<local::range, global::range>>
      gets ( expr::eval::context const& input
           , we::type::module_call_t const& module_call
           )
    {
      std::list<std::pair<local::range, global::range>> gets;

      for (type::memory_transfer const& mg : module_call.memory_gets())
      {
        expr::eval::context context (input);

        for ( std::pair<local::range, global::range> const& transfer
            : zip ( evaluate<local::range> (context, mg.local())
                  , evaluate<global::range> (context, mg.global())
                  )
            )
        {
          gets.emplace_back (transfer);
        }
      }

      return gets;
    }

    std::list<std::pair<local::range, global::range>>
      puts ( expr::eval::context const& output
           , we::type::module_call_t const& module_call
           )
    {
      std::list<std::pair<local::range, global::range>> puts;

      for (type::memory_transfer const& mp : module_call.memory_puts())
      {
        expr::eval::context context (output);

        for ( std::pair<local::range, global::range> const& transfer
            : zip ( evaluate<local::range> (context, mp.local())
                  , evaluate<global::range> (context, mp.global())
                  )
            )
        {
          puts.emplace_back (transfer);
        }
      }

      return puts;
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
        (fvmGetGlobalData, memory_buffer, gets (input (act), module_call));

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

      transfer (fvmPutGlobalData, memory_buffer, puts (out, module_call));
    }
  }
}
