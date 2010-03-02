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
 *       Filename:  LocatorPersister.cpp
 *
 *    Description:  implements a file persister
 *
 *        Version:  1.0
 *        Created:  10/24/2009 12:09:24 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "LocatorPersister.hpp"
#include <fstream>
#include <fhglog/fhglog.hpp>

namespace seda { namespace comm {
  void LocatorPersister::load(Locator &locator) const
  {
    std::ifstream ifs(path().c_str(), std::ios_base::in | std::ios_base::binary);
    if (! ifs)
    {
      LOG(ERROR, path() << " could not be opened for writing!");
      throw CommunicationError("could not open file for writing");  
    }
    std::string line;
    while (ifs)
    {
      std::getline(ifs, line);
      if (line.empty() || line[0] == '#')
        continue;
      DLOG(DEBUG, "loading location from string: " << line);

      std::string::size_type sep1_idx = line.find_first_of(':', 0);
      std::string::size_type sep2_idx = line.find_first_of(':', sep1_idx+1);

      std::string name(line.substr(0,          0+sep1_idx));
      std::string host(line.substr(sep1_idx+1, sep2_idx - (sep1_idx+1)));
      std::string port(line.substr(sep2_idx+1));
      DLOG(DEBUG, "name = " << name << " host = " << host << " port = " << port);
      locator.insert(name, host+":"+port);
    }
  }

  void LocatorPersister::store(const Locator &locator) const
  {
    std::ofstream ofs(path().c_str(), std::ios_base::trunc | std::ios_base::binary);
    if (! ofs)
    {
      LOG(ERROR, path() << " could not be opened for writing!");
      throw CommunicationError("could not open file for writing");  
    }

    for (Locator::const_iterator loc_it(locator.begin())
       ; loc_it != locator.end()
       ; ++loc_it)
    {
      const Locator::location_t &loc = loc_it->second;
      DLOG(DEBUG, "storing location: " << loc.name() << " -> " << loc.host() << ":" << loc.port());
      ofs << loc.name() << ':' << loc.host() << ":" << loc.port() << std::endl;
    }
  }
}}
