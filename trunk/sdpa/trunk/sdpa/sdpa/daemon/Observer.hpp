/*
 * =====================================================================================
 *
 *       Filename:  Observer.hpp
 *
 *    Description:  defines the interface for the observer pattern
 *
 *        Version:  1.0
 *        Created:  11/19/2009 10:36:51 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_DAEMON_OBSERVER_HPP
#define SDPA_DAEMON_OBSERVER_HPP 1

#include <boost/any.hpp>

namespace sdpa { namespace daemon {
  class Observer
  {
  public:
    virtual ~Observer() {}

    virtual void update(const boost::any &) = 0;
  };
}}

#endif
