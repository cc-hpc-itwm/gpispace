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

#ifndef WE_TYPE_MODULE_CALL_HPP
#define WE_TYPE_MODULE_CALL_HPP 1

#include <we/type/module_call.fwd.hpp>

#include <ostream>
#include <boost/serialization/nvp.hpp>

#include <unordered_map>

namespace we { namespace type {
  struct module_call_t
  {
    module_call_t () = default;

    module_call_t
      ( const std::string & module
      , const std::string & function
      , std::unordered_map<std::string, std::string>&& memory_buffers
      )
      : module_(module)
      , function_(function)
      , _memory_buffers (memory_buffers)
    {}

    const std::string & module () const { return module_; }
    const std::string & function () const { return function_; }

    std::unordered_map<std::string, std::string> const& memory_buffers() const
    {
      return _memory_buffers;
    }

    private:
    std::string module_;
    std::string function_;
    std::unordered_map<std::string, std::string> _memory_buffers;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(module_);
      ar & BOOST_SERIALIZATION_NVP(function_);
      ar & BOOST_SERIALIZATION_NVP (_memory_buffers);
    }
  };

  inline std::ostream & operator << (std::ostream & os, const module_call_t & m)
  {
    return os << m.module() << "." << m.function();
  }
}}

#endif
