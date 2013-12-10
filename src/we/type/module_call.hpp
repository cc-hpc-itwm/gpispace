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

namespace we { namespace type {
  struct module_call_t
  {
    module_call_t ()
    {}

    module_call_t (const std::string & module, const std::string & function)
      : module_(module)
      , function_(function)
    {}

    const std::string & module () const { return module_; }
    const std::string & function () const { return function_; }

    void module (const std::string & m) { module_ = m; }
    void function (const std::string & f) { function_ = f; }
    private:
    std::string module_;
    std::string function_;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(module_);
      ar & BOOST_SERIALIZATION_NVP(function_);
    }
  };

  inline std::ostream & operator << (std::ostream & os, const module_call_t & m)
  {
    return os << m.module() << "." << m.function();
  }
}}

#endif
