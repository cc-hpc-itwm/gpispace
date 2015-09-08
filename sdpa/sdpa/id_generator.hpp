#pragma once

#include <util-generic/hostname.hpp>
#include <util-generic/syscall.hpp>

#include <boost/format.hpp>

#include <atomic>

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
      , _prefix ( ( boost::format ("%1%.%2%.%3%.")
                  % fhg::util::hostname()
                  % name
                  % fhg::util::syscall::getpid()
                  ).str()
                )
    {}

  private:
    std::atomic<std::size_t> _counter;
    std::string _prefix;
  };
}
