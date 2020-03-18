#pragma once

#include <gpi-space/types.hpp>

#include <boost/any.hpp>

#include <ostream>
#include <string>
#include <vector>

namespace fhg
{
  namespace vmem
  {
    //! Wrapper for gpi::netdev_id_t with to/from string support for
    //! command line option parsing.
    struct netdev_id
    {
      //! Let GASPI automatically choose netdev id.
      netdev_id();
      //! Depending on \a option either choose automatically ("auto"),
      //! device 0 ("0") or device 1 ("1").
      netdev_id (std::string const& option);
      //! Convert the current choice in the same way that the \c option
      //! constructor overload expects it.
      friend std::string to_string (netdev_id const&);
      //! Output \c to_string().
      friend std::ostream& operator<< (std::ostream&, netdev_id const&);
      //! Validate a command line option in the Boost.ProgramOptions
      //! framework.
      friend void validate
        (boost::any&, std::vector<std::string> const&, netdev_id*, int);
      //! Serialize using Boost.Serialization.
      template<typename Archive>
        void serialize (Archive&, unsigned int);

      //! The choice in a format that can be written to \c gaspi_config_t.
      gpi::netdev_id_t value;
    };
  }
}

#include <vmem/netdev_id.ipp>
