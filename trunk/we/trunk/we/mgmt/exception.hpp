/*
 * =====================================================================================
 *
 *       Filename:  exception.hpp
 *
 *    Description:  exception definitions
 *
 *        Version:  1.0
 *        Created:  03/15/2010 01:31:17 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_LAYER_EXCEPTION_HPP
#define WE_MGMT_LAYER_EXCEPTION_HPP 1

#include <stdexcept>

namespace we { namespace mgmt {
  template <typename Id>
  struct activity_not_found : std::runtime_error
  {
    typedef Id id_type;

    activity_not_found (std::string const& msg, id_type const& id_)
      : std::runtime_error(msg)
      , id(id_)
    {}
      
    virtual ~activity_not_found() throw() {}

    const id_type id;
  };
}}

#endif
