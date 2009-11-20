/*
 * =====================================================================================
 *
 *       Filename:  types.hpp
 *
 *    Description:  type definitions for the client
 *
 *        Version:  1.0
 *        Created:  11/04/2009 02:34:43 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_CLIENT_TYPES_HPP
#define SDPA_CLIENT_TYPES_HPP 1

#include <string>
#include <sdpa/types.hpp>
#include <sdpa/util/Config.hpp>

namespace sdpa { namespace client {
  typedef sdpa::util::NewConfig config_t;
  typedef sdpa::job_result_t result_t;
}}

#endif
