#include <we/loader/module_call.hpp>

#include <we/type/id.hpp>
#include <we/type/port.hpp>
#include <we/expr/parse/parser.hpp>

//! \todo remove, needed to make a complete type
#include <we/type/net.hpp>

namespace we
{
  namespace
  {
    expr::eval::context input (we::type::activity_t const& activity)
    {
      expr::eval::context context;

      for (auto const& token_on_port : activity.input())
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

    class memory_buffer : boost::noncopyable
    {
    public:
      memory_buffer ( we::type::activity_t const& activity
                    , std::string const& expression
                    )
        : _size (evaluate_size_or_die (activity, expression))
        , _buffer (new char[_size])
      {}

      ~memory_buffer()
      {
        delete _buffer;
      }

      void* ptr() const
      {
        return static_cast<void*> (_buffer);
      }
      unsigned long size() const
      {
        return _size;
      }

    private:
      unsigned long _size;
      char* _buffer;
    };
  }

  namespace loader
  {
    void module_call ( we::loader::loader& loader
                     , drts::worker::context* context
                     , we::type::activity_t& act
                     , const we::type::module_call_t& module_call
                     )
    {
      std::list<memory_buffer> buffers;
      std::map<std::string, void*> pointers;

      for (auto const& buffer_and_size : module_call.memory_buffers())
      {
        buffers.emplace_back (act, buffer_and_size.second);
        pointers.emplace (buffer_and_size.first, buffers.back().ptr());
      }

      typedef std::pair< we::port_id_type
                       , we::type::port_t
                       > port_by_id_type;

      expr::eval::context out;

      loader[module_call.module()].call
        (module_call.function(), context, input (act), out, pointers);

      for (const port_by_id_type& port_by_id : act.transition().ports_output())
      {
        const we::port_id_type& port_id (port_by_id.first);
        const we::type::port_t& port (port_by_id.second);

        act.add_output (port_id, out.value (port.name()));
      }
    }
  }
}
