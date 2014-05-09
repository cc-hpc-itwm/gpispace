#include <we/loader/module_call.hpp>

#include <we/type/id.hpp>
#include <we/type/port.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/field.hpp>

//! \todo remove, needed to make a complete type
#include <we/type/net.hpp>

#include <unordered_map>

#include <iostream>

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

    namespace global
    {
      struct handle
      {
      public:
        handle (pnet::type::value::value_type const& value)
          : _name ( pnet::field_as<std::string>
                    ("name", value, std::string ("string"))
                  )
        {}
        std::string name() const
        {
          return _name;
        }
      private:
        std::string _name;
      };

      struct range
      {
      public:
        range (pnet::type::value::value_type const& value)
          : _handle (pnet::field ("handle", value, std::string ("handle")))
          , _offset ( pnet::field_as<unsigned long>
                      ("offset", value, std::string ("unsigned long"))
                    )
          , _size ( pnet::field_as<unsigned long>
                    ("size", value, std::string ("unsigned long"))
                  )
        {}
        struct handle const& handle() const
        {
          return _handle;
        }
        unsigned long offset() const
        {
          return _offset;
        }
        unsigned long size() const
        {
          return _size;
        }

      private:
        struct handle _handle;
        unsigned long _offset;
        unsigned long _size;
      };
    }

    namespace local
    {
      struct range
      {
      public:
        range (pnet::type::value::value_type const& value)
          : _buffer ( pnet::field_as<std::string>
                      ("buffer", value, std::string ("string"))
                    )
          , _offset ( pnet::field_as<unsigned long>
                      ("offset", value, std::string ("unsigned long"))
                    )
          , _size ( pnet::field_as<unsigned long>
                    ("size", value, std::string ("unsigned long"))
                  )
        {}
        std::string const& buffer() const
        {
          return _buffer;
        }
        unsigned long offset() const
        {
          return _offset;
        }
        unsigned long size() const
        {
          return _size;
        }

      private:
        std::string _buffer;
        unsigned long _offset;
        unsigned long _size;
      };
    }

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

    void transfer
      ( std::string const& head
      , std::string const& sep
      , std::unordered_map<std::string, memory_buffer*> memory_buffer
      , expr::eval::context& context
      , std::string const& global
      , std::string const& local
      )
    {
      std::cout << head << ' ';

      for (local::range const& r : evaluate<local::range> (context, local))
      {
        std::cout << "{" << r.buffer()
                  << ", " << r.offset()
                  << ", " << r.size()
                  << "}";

        if (!memory_buffer.count (r.buffer()))
        {
          //! \todo specific exception
          throw std::runtime_error ("unknown memory buffer " + r.buffer());
        }

        if (r.offset() + r.size() > memory_buffer.at (r.buffer())->size())
        {
          //! \todo specific exception
          throw std::runtime_error ("local range to large");
        }
      }

      std::cout << ' ' << sep << ' ';

      for (global::range const& r : evaluate<global::range> (context, global))
      {
        std::cout << "{" << r.handle().name()
                  << ", " << r.offset()
                  << ", " << r.size()
                  << "}";

        //! \todo check range, needs knowledge about global memory
      }

      std::cout << std::endl;
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
      std::list<memory_buffer> buffers;
      std::map<std::string, void*> pointers;
      std::unordered_map<std::string, memory_buffer*> memory_buffer;

      for (auto const& buffer_and_size : module_call.memory_buffers())
      {
        buffers.emplace_back (act, buffer_and_size.second);
        pointers.emplace (buffer_and_size.first, buffers.back().ptr());
        memory_buffer.emplace (buffer_and_size.first, &buffers.back());
      }

      for (type::memory_transfer const& mg : module_call.memory_gets())
      {
        expr::eval::context context (input (act));

        transfer ("GET", "<<", memory_buffer, context, mg.global(), mg.local());
      }

      //! \todo wait

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

      for (type::memory_transfer const& mp : module_call.memory_puts())
      {
        expr::eval::context context (out);

        transfer ("PUT", ">>", memory_buffer, context, mp.global(), mp.local());
      }

      //! \todo wait
    }
  }
}
