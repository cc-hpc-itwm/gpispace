// alexander.petry@itwm.fraunhofer.de

#pragma once

#include <util-generic/hostname.hpp>
#include <util-generic/syscall.hpp>

#include <boost/format.hpp>

#include <atomic>
#include <chrono>

namespace sdpa {
  class id_generator
  {
  public:
    std::string next()
    {
      return _prefix + std::to_string (_counter.fetch_add (1));
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
    std::atomic<std::size_t> _counter;
    std::string _prefix;
  };
}
