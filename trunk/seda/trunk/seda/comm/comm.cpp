/*
 * =====================================================================================
 *
 *       Filename:  comm.cpp
 *
 *    Description:  implementation
 *
 *        Version:  1.0
 *        Created:  10/26/2009 03:55:53 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "comm.hpp"

namespace seda { namespace comm {
  void initialize()
  {

  }

  void shutdown()
  {
  }

  const Locator::ptr_t &globalLocator()
  {
    static Locator::ptr_t global_locator(new seda::comm::Locator());
    return global_locator;
  }
}}
