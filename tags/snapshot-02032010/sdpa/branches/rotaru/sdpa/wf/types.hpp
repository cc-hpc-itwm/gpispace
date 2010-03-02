/*
 * =====================================================================================
 *
 *       Filename:  types.hpp
 *
 *    Description:  typedefinitions
 *
 *        Version:  1.0
 *        Created:  11/15/2009 04:52:04 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_WF_TYPES_HPP
#define SDPA_WF_TYPES_HPP 1

#include <map>
#include <sdpa/wf/Parameter.hpp>

namespace sdpa { namespace wf {
  typedef sdpa::wf::Parameter parameter_t;
  typedef std::map<std::string, parameter_t> parameters_t;
}}

#endif
