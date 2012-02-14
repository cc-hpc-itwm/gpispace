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
 *       Filename:  ConnectionFactory.cpp
 *
 *    Description:  implementation of the connectionfactory
 *
 *        Version:  1.0
 *        Created:  09/08/2009 04:15:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "comm.hpp"
#include "ConnectionFactory.hpp"
#include "UDPConnection.hpp"

using namespace seda::comm;

ConnectionParameters ConnectionFactory::parse_uri(const std::string &uri)
{
  ConnectionParameters params("tcp", "localhost", "foobar");
  params.put("protocol", "zmq");
  params.put("uri", uri);
  return params;
}

Connection::ptr_t ConnectionFactory::createConnection(const ConnectionParameters &params)
{
  return createConnection(params, globalLocator());
}


Connection::ptr_t ConnectionFactory::createConnection(const ConnectionParameters &params, const Locator::ptr_t &locator)
{
  if (params.transport() == "udp")
  {
    return Connection::ptr_t(
      new UDPConnection(
          locator
        , params.name()
        , params.host()
        , params.port()
      )
    );
  }
  throw std::runtime_error(std::string("no suitable connection found for protocol: ") + params.transport());
}

