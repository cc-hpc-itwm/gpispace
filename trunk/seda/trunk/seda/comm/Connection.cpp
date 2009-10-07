/*
 * =====================================================================================
 *
 *       Filename:  Connection.cpp
 *
 *    Description:  implementation of the generic connection parts
 *
 *        Version:  1.0
 *        Created:  09/09/2009 10:10:45 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "Connection.hpp"
#include <iostream>

using namespace seda::comm;

void Connection::registerListener(ConnectionListener *listener)
{
  listener_list_.push_back(listener);
}

void Connection::removeListener(ConnectionListener *listener)
{
  listener_list_.push_back(listener);
}

void Connection::notifyListener(const seda::comm::SedaMessage &msg) const
{
  for (listener_list_t::const_iterator listener(listener_list_.begin())
     ; listener != listener_list_.end()
     ; ++listener)
  {
    try
    {
      (*listener)->onMessage(msg);
    }
    catch (...)
    {
      std::cerr << "FIXME: connection listener bailed out, introduce error logging here: " << __FILE__ << ":" << __LINE__ << std::endl;
    }
  }
}
