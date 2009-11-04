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
#include <boost/program_options.hpp>

namespace sdpa { namespace client {
  typedef boost::program_options::variables_map config_t;
  typedef std::string result_t;
}}

#endif
