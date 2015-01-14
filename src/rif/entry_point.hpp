#ifndef FHG_RIF_ENTRY_POINT_HPP
#define FHG_RIF_ENTRY_POINT_HPP

#include <boost/serialization/split_free.hpp>

#include <string>

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

#endif
