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
 *       Filename:  Serialization.hpp
 *
 *    Description:  serialization code
 *
 *        Version:  1.0
 *        Created:  10/28/2009 01:12:12 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_SERIALIZATION_HPP
#define SEDA_COMM_SERIALIZATION_HPP 1

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <seda/comm/SedaMessage.hpp>

namespace boost { namespace serialization {
  template <class Archive>
  void serialize(Archive & ar, seda::comm::SedaMessage & msg, const unsigned int version)
  {
    ar & msg.from();
    ar & msg.to();
    ar & msg.payload();

	if (version > 0)
	  ar & msg.id();
  }
}}

BOOST_CLASS_VERSION(seda::comm::SedaMessage, 1)

#endif
