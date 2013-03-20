/*
 * =====================================================================================
 *
 *       Filename:  id_generator.hpp
 *
 *    Description:  generates message ids
 *
 *        Version:  1.0
 *        Created:  12/13/2009 08:27:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_EVENTS_ID_GENERATOR_HPP
#define SDPA_EVENTS_ID_GENERATOR_HPP 1

#include <boost/thread.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace sdpa { namespace events {
  class id_generator
  {
  private:
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;

  public:
	static id_generator& instance()
	{
	  static id_generator gen;
	  return gen;
	}

	std::string next()
	{
	  lock_type lock(mtx_);
	  return boost::uuids::to_string (boost::uuids::random_generator()());
	}

  private:
	id_generator()
	{}

	mutex_type mtx_;
  };
}}

#endif
