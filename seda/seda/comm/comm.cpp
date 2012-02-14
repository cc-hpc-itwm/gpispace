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
#include <fhglog/fhglog.hpp>

namespace seda { namespace comm {
  void initialize(int /* argc */, char **  /* argv */)
  {
    DLOG(DEBUG, "initializing seda-comm library");
  }

  void shutdown()
  {
    DLOG(DEBUG, "shutting seda-comm library down");
  }

  const Locator::ptr_t &globalLocator()
  {
    static Locator::ptr_t global_locator(new seda::comm::Locator());
    return global_locator;
  }
}}
