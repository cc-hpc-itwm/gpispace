#pragma once

#include <boost/serialization/split_free.hpp>

#include <util-generic/ostream/modifier.hpp>

#include <string>
#include <stdexcept>
#include <sstream>
#include <tuple>

#include <unistd.h>

namespace fhg
{
  namespace iml
  {
    namespace rif
    {
      struct entry_point : public fhg::util::ostream::modifier
      {
        std::string hostname;
        unsigned short port;
        pid_t pid;

        //! \note Serialization only.
        entry_point() = default;

        entry_point (std::string const& hostname_, unsigned short port_, pid_t pid_)
          : hostname (hostname_)
          , port (port_)
          , pid (pid_)
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
        virtual std::ostream& operator() (std::ostream& os) const override
        {
          return os << hostname << ' ' << port << ' ' << pid;
        }

        bool operator== (entry_point const& other) const
        {
          return std::tie (hostname, port, pid)
            == std::tie (other.hostname, other.port, other.pid);
        }
      };
    }
  }
}

namespace std
{
  template<>
    struct hash<fhg::iml::rif::entry_point>
  {
    std::size_t operator() (const fhg::iml::rif::entry_point& ep) const
    {
      return std::hash<std::string>() (ep.string());
    }
  };
}

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void load ( Archive& ar
                , fhg::iml::rif::entry_point& entry_point
                , const unsigned int
                )
    {
      std::string hostname;
      unsigned short port;
      pid_t pid;
      ar & hostname;
      ar & port;
      ar & pid;
      entry_point = fhg::iml::rif::entry_point (hostname, port, pid);
    }
    template<typename Archive>
      void save ( Archive& ar
                , fhg::iml::rif::entry_point const& entry_point
                , const unsigned int
                )
    {
      ar & entry_point.hostname;
      ar & entry_point.port;
      ar & entry_point.pid;
    }

    template<typename Archive>
      void serialize ( Archive& ar
                     , fhg::iml::rif::entry_point& entry_point
                     , const unsigned int version
                     )
    {
      boost::serialization::split_free (ar, entry_point, version);
    }
  }
}
