/*
 * =====================================================================================
 *
 *       Filename:  Locator.hpp
 *
 *    Description:  locator service
 *
 *        Version:  1.0
 *        Created:  10/23/2009 02:05:33 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_LOCATOR_HPP
#define SEDA_COMM_LOCATOR_HPP 1

#include <fhglog/fhglog.hpp>
#include <seda/shared_ptr.hpp>
#include <string>
#include <map>

namespace seda { namespace comm {
  class Locator
  {
  public:
    typedef shared_ptr<Locator> ptr_t;
    typedef std::pair<std::string, short> location_t;
    typedef std::map<std::string, location_t> location_map_t;
    

    const location_t &lookup(const std::string &name) const throw (std::exception)
    {
      DLOG(DEBUG, "looking up " << name);
      location_map_t::const_iterator loc_it(locations_.find(name));
      if (loc_it != locations_.end())
      {
        const location_t &loc(loc_it->second);
        DLOG(DEBUG, "location of " << name << " is " << loc.first << ":" << loc.second);
        return loc;
      }
      else
      {
        DLOG(DEBUG, "not found");
        throw std::runtime_error("could not locate " + name);
      }
    }

    void insert(const std::string &name, const std::string &h, short p)
    {
      insert(name, std::make_pair(h,p));
    }
    void insert(const std::string &name, const location_t &location)
    {
      DLOG(DEBUG, "updating location of " << name << " to " << location.first << ":" << location.second);
      locations_[name] = location;
    }

    void remove(const std::string &name)
    {
      locations_.erase(name);
    }
  private:
    location_map_t locations_;    
  };
}}

#endif
