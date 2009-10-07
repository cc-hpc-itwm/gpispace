/*
 * =====================================================================================
 *
 *       Filename:  ConnectionListener.hpp
 *
 *    Description:  a conneciton listener interface
 *
 *        Version:  1.0
 *        Created:  09/08/2009 03:27:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_CONNECTION_LISTENER_HPP
#define SEDA_COMM_CONNECTION_LISTENER_HPP

#include <seda/comm/SedaMessage.hpp>

namespace seda { namespace comm {
  class ConnectionListener {
  public:
    virtual void onMessage(const seda::comm::SedaMessage &) = 0;
  };
}}

#endif
