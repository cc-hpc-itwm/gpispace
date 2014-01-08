/*
 * =============================================================================
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
 * =============================================================================
 */

#ifndef SDPA_ID_GENERATOR_HPP
#define SDPA_ID_GENERATOR_HPP 1

#include <fhg/util/hostname.hpp>
#include <fhg/util/counter.hpp>
#include <sys/types.h> // pid_t
#include <unistd.h> // getpid
#include <string.h> // memset
#include <time.h>   // time
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace sdpa {
  class id_generator
  {
  public:

  template <class Tag>
	static id_generator& instance()
	{
	  static id_generator gen (Tag::name());
	  return gen;
	}

	std::string next()
	{
    std::size_t id;
    {
      boost::mutex::scoped_lock const _ (_counter_mutex);
      id = _counter++;
    }
    return _prefix + boost::lexical_cast<std::string> (id);
  }
  private:
    id_generator (std::string const& name)
      : _counter()
      , _prefix ( ( boost::format ("%1%.%2%.%3%.%4%.")
                  % fhg::util::hostname()
                  % name
                  % time (0)
                  % getpid()
                  ).str()
                )
    {}

    mutable boost::mutex _counter_mutex;
    std::size_t _counter;
    std::string _prefix;
  };
}

#endif
