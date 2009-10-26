/*
 * =====================================================================================
 *
 *       Filename:  comm.hpp
 *
 *    Description:  global include file that also provides init/shutdown for the library
 *
 *        Version:  1.0
 *        Created:  10/26/2009 03:53:25 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_COMM_HPP
#define SEDA_COMM_COMM_HPP 1

#include <seda/comm/Locator.hpp>

namespace seda { namespace comm {
  extern void initialize();
  extern void shutdown();

  extern const Locator::ptr_t &globalLocator();
}}

#endif
