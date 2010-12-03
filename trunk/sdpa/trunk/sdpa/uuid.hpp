#ifndef SDPA_UUID_HPP
#define SDPA_UUID_HPP 1

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace sdpa {
  struct uuid {
	  boost::uuids::uuid uid_;
	  uuid() {}
	  uuid( const boost::uuids::uuid& uid) : uid_(uid) {}
	  std::string str() { return boost::lexical_cast<std::string>(uid_); }
  };
}

#endif
