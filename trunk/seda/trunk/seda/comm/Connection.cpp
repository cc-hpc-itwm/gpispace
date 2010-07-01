/*
   Copyright (C) 2009 Alexander Petry <alexander.petry@itwm.fraunhofer.de>.

   This file is part of seda.

   seda is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   seda is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with seda; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

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

void Connection::set_option(option::enable_compression const &compress)
{
  LOG_IF_ELSE(INFO, compress, "compression enabled", "compression disabled");
  compression_enabled_ = compress;
}

void Connection::get_option(option::enable_compression & opt)
{
  opt.set(compression_enabled_);
}
