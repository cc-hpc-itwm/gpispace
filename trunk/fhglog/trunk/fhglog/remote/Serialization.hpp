/*
 * =====================================================================================
 *
 *       Filename:  Serialization.hpp
 *
 *    Description:  serialization functions for logevents
 *
 *        Version:  1.0
 *        Created:  10/19/2009 01:17:25 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHG_LOG_REMOTE_SERIALIZATION_HPP
#define FHG_LOG_REMOTE_SERIALIZATION_HPP 1

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <fhglog/LogLevel.hpp>
#include <fhglog/LogEvent.hpp>

namespace boost { namespace serialization {
  template <class Archive>
  void serialize(Archive & ar, fhg::log::LogLevel & level, const unsigned int /* version */)
  {
    ar & level.lvl();
  }

  template <class Archive>
  void serialize(Archive & ar, fhg::log::LogEvent & event, const unsigned int /* version */)
  {
    ar & event.severity();
    ar & event.file();
    ar & event.path();
    ar & event.function();
    ar & event.line();
    ar & event.message();
    ar & event.tstamp();
    ar & event.pid();
    ar & event.tid();
    ar & event.logged_via();
    ar & event.logged_on();
  }
}}

#endif
