// alexander.petry@itwm.fraunhofer.de

#pragma once

#include <util-generic/hostname.hpp>
#include <util-generic/syscall.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <chrono>

namespace sdpa {
  class id_generator
  {
  public:
	std::string next()
	{
      boost::mutex::scoped_lock const _ (_counter_mutex);
    return _prefix + boost::lexical_cast<std::string> (_counter++);
  }

    id_generator (std::string const& name)
      : _counter()
      , _prefix ( ( boost::format ("%1%.%2%.%3%.%4%.")
                  % fhg::util::hostname()
                  % name
                  % std::chrono::duration_cast<std::chrono::seconds>
                      ( std::chrono::steady_clock::now().time_since_epoch()
                      ).count()
                  % fhg::util::syscall::getpid()
                  ).str()
                )
    {}

  private:
    mutable boost::mutex _counter_mutex;
    std::size_t _counter;
    std::string _prefix;
  };
}
