#pragma once

#include <we/type/module_call.fwd.hpp>

#include <we/expr/eval/context.hpp>
#include <we/type/memory_buffer_info_t.hpp>
#include <we/type/memory_transfer.hpp>
#include <we/type/range.hpp>

#include <ostream>
#include <boost/serialization/nvp.hpp>

#include <algorithm>
#include <list>
#include <numeric>
#include <unordered_map>

namespace we { namespace type {
  struct module_call_t
  {
    module_call_t () = default;

    module_call_t
      ( const std::string & module
      , const std::string & function
      , std::unordered_map<std::string, memory_buffer_info_t>&& memory_buffers
      , std::list<memory_transfer>&& memory_gets
      , std::list<memory_transfer>&& memory_puts
      , bool require_function_unloads_without_rest
      , bool require_module_unloads_without_rest
      )
      : module_(module)
      , function_(function)
      , _memory_buffers (memory_buffers)
      , _memory_gets (memory_gets)
      , _memory_puts (memory_puts)
      , _require_function_unloads_without_rest
          (require_function_unloads_without_rest)
      , _require_module_unloads_without_rest
          (require_module_unloads_without_rest)
    {}

    const std::string & module () const { return module_; }
    const std::string & function () const { return function_; }

    bool require_function_unloads_without_rest() const;
    bool require_module_unloads_without_rest() const;

    std::unordered_map<std::string, memory_buffer_info_t> const& memory_buffers() const
    {
      return _memory_buffers;
    }
    std::list<memory_transfer> const& memory_gets() const
    {
      return _memory_gets;
    }
    std::list<memory_transfer> const& memory_puts() const
    {
      return _memory_puts;
    }

    std::list<std::pair<local::range, global::range>>
      puts_evaluated_before_call (expr::eval::context const&) const;
    std::list<std::pair<local::range, global::range>>
      puts_evaluated_after_call (expr::eval::context const&) const;
    std::list<std::pair<local::range, global::range>>
      gets (expr::eval::context const&) const;

    std::unordered_map<std::string, unsigned long>
      memory_buffer_sizes (expr::eval::context const&) const;

    unsigned long memory_buffer_size_total
      (expr::eval::context const& context) const
    {
      std::unordered_map<std::string, unsigned long> const sizes
        (memory_buffer_sizes (context));

      return std::accumulate
        ( sizes.begin(), sizes.end(), 0UL
        , [](unsigned long a, std::pair<std::string, unsigned long> const& x)
          {
            return a + x.second;
          }
        );
    }

    private:
    std::string module_;
    std::string function_;
    std::unordered_map<std::string, memory_buffer_info_t> _memory_buffers;
    std::list<memory_transfer> _memory_gets;
    std::list<memory_transfer> _memory_puts;
    bool _require_function_unloads_without_rest;
    bool _require_module_unloads_without_rest;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(module_);
      ar & BOOST_SERIALIZATION_NVP(function_);
      ar & BOOST_SERIALIZATION_NVP (_memory_buffers);
      ar & BOOST_SERIALIZATION_NVP (_memory_gets);
      ar & BOOST_SERIALIZATION_NVP (_memory_puts);
      ar & BOOST_SERIALIZATION_NVP (_require_function_unloads_without_rest);
      ar & BOOST_SERIALIZATION_NVP (_require_module_unloads_without_rest);
    }
  };

  inline std::ostream & operator << (std::ostream & os, const module_call_t & m)
  {
    return os << m.module() << "." << m.function();
  }
}}
