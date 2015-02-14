#pragma once

#include <boost/serialization/split_free.hpp>

#include <string>
#include <stdexcept>
#include <sstream>
#include <tuple>

#include <unistd.h>

namespace fhg
{
  namespace rif
  {
    struct entry_point
    {
      std::string hostname;
      unsigned short port;
      pid_t pid;

      //! \note Serialization only.
      entry_point() = default;

      entry_point (std::string const& hostname, unsigned short port, pid_t pid)
        : hostname (hostname)
        , port (port)
        , pid (pid)
      {}

      entry_point (std::string const& input)
      {
        std::istringstream iss (input);
        if (!(iss >> hostname >> port >> pid))
        {
          throw std::runtime_error
            ("parse error: expected 'host port pid': got '" + input + "'");
        }
      }
      std::string to_string() const
      {
        std::ostringstream oss;
        oss << hostname << ' ' << port << ' ' << pid;
        return oss.str();
      }

      bool operator== (entry_point const& other) const
      {
        return std::tie (hostname, port, pid)
          == std::tie (other.hostname, other.port, other.pid);
      }
    };
  }
}

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void load ( Archive& ar
                , fhg::rif::entry_point& entry_point
                , const unsigned int
                )
    {
      std::string hostname;
      unsigned short port;
      pid_t pid;
      ar & hostname;
      ar & port;
      ar & pid;
      entry_point = fhg::rif::entry_point (hostname, port, pid);
    }
    template<typename Archive>
      void save ( Archive& ar
                , fhg::rif::entry_point const& entry_point
                , const unsigned int
                )
    {
      ar & entry_point.hostname;
      ar & entry_point.port;
      ar & entry_point.pid;
    }

    template<typename Archive>
      void serialize ( Archive& ar
                     , fhg::rif::entry_point& entry_point
                     , const unsigned int version
                     )
    {
      boost::serialization::split_free (ar, entry_point, version);
    }
  }
}
