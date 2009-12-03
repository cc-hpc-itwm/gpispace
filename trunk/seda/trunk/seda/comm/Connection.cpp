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
  boost::unique_lock<boost::recursive_mutex> lock(listener_mtx_);
  listener_list_.push_back(listener);
}

void Connection::removeListener(ConnectionListener *listener)
{
  boost::unique_lock<boost::recursive_mutex> lock(listener_mtx_);
  listener_list_.remove(listener);
}

bool Connection::has_listeners() const
{
  boost::unique_lock<boost::recursive_mutex> lock(const_cast<boost::recursive_mutex&>(listener_mtx_));
  return ! listener_list_.empty();
}

void Connection::notifyListener(const seda::comm::SedaMessage &msg)
{
  boost::unique_lock<boost::recursive_mutex> lock(listener_mtx_);
  for (listener_list_t::const_iterator listener(listener_list_.begin())
     ; listener != listener_list_.end()
     ; ++listener)
  {
    try
    {
      (*listener)->onMessage(msg);
    }
    catch (const std::exception &ex)
    {
	  LOG(ERROR, "connection listener onMessage() failed: " << ex.what());
    }
    catch (...)
    {
	  LOG(ERROR, "connection listener onMessage() failed with an unknown reason!");
    }
  }
}
