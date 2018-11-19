/*
 * =====================================================================================
 *
 *       Filename:  module_call.hpp
 *
 *    Description:  module call descriptor
 *
 *        Version:  1.0
 *        Created:  04/13/2010 12:22:29 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#pragma once

#include <we/type/module_call.fwd.hpp>

#include <we/expr/eval/context.hpp>
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
      , std::unordered_map<std::string, std::string>&& memory_buffers
      , std::unordered_map<std::string, std::string>&& cached_memory_buffers_with_dataids
      , std::list<memory_transfer>&& memory_gets
      , std::list<memory_transfer>&& memory_puts
      )
      : module_(module)
      , function_(function)
      , _memory_buffers (memory_buffers)
      , _cached_memory_buffers_with_dataids(cached_memory_buffers_with_dataids)
      , _memory_gets (memory_gets)
      , _memory_puts (memory_puts)
    {}

    const std::string & module () const { return module_; }
    const std::string & function () const { return function_; }

    std::unordered_map<std::string, std::string> const& memory_buffers() const
    {
      return _memory_buffers;
    }
    std::unordered_map<std::string, std::string> const& memory_buffer_dataids() const
    {
      return _cached_memory_buffers_with_dataids;
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
    std::unordered_map<std::string, std::string> _memory_buffers;
    std::unordered_map<std::string, std::string> _cached_memory_buffers_with_dataids;
    std::list<memory_transfer> _memory_gets;
    std::list<memory_transfer> _memory_puts;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(module_);
      ar & BOOST_SERIALIZATION_NVP(function_);
      ar & BOOST_SERIALIZATION_NVP (_memory_buffers);
      ar & BOOST_SERIALIZATION_NVP (_cached_memory_buffers_with_dataids);
      ar & BOOST_SERIALIZATION_NVP (_memory_gets);
      ar & BOOST_SERIALIZATION_NVP (_memory_puts);
    }
  };

  inline std::ostream & operator << (std::ostream & os, const module_call_t & m)
  {
    return os << m.module() << "." << m.function();
  }
}}
