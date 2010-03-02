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
 *       Filename:  ConnectionStrategy.cpp
 *
 *    Description:  implementation of the seda <-> comm gateway
 *
 *        Version:  1.0
 *        Created:  09/09/2009 10:04:15 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "ConnectionStrategy.hpp"

using namespace seda::comm;

void ConnectionStrategy::perform(const IEvent::Ptr &toSend)
{
  if (seda::comm::SedaMessage *msg = dynamic_cast<seda::comm::SedaMessage*>(toSend.get()))
  {
    conn_->send(*msg);
  }
  else
  {
    LOG(ERROR,"could not send event (does not inherit SedaMessage): " << toSend->str());
  }
}

void ConnectionStrategy::onMessage(const seda::comm::SedaMessage &recvMsg)
{
  DMLOG(TRACE, "got message: " << recvMsg.str());
  ForwardStrategy::perform(SedaMessage::Ptr(new SedaMessage(recvMsg)));
}
