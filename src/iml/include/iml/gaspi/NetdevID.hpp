// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/detail/dllexport.hpp>

#include <ostream>
#include <string>
#include <vector>

namespace boost
{
  class any;
}

namespace iml
{
  namespace gaspi
  {
    //! Wrapper for GPI's netdev_id_t with to/from string support for
    //! command line option parsing.
    struct IML_DLLEXPORT NetdevID
    {
      //! Depending on \a option either choose automatically ("auto"),
      //! device 0 ("0"), device 1 ("1"), ... .
      NetdevID (std::string const& option = "auto");

      //! Convert the option string to a device ID.
      //! The option string can be either "auto" or any integer >= -1.
      static int from_string(std::string const& option);

      //! Convert the current choice in the same way that the \c option
      //! constructor overload expects it.
      std::string to_string() const;
      //! Output \c to_string().
      IML_DLLEXPORT
        friend std::ostream& operator<< (std::ostream&, NetdevID const&);

      //! Serialize using Boost.Serialization.
      template<typename BoostArchive>
        void serialize (BoostArchive&, unsigned int);

      //! The choice in a format that can be written to \c gaspi_config_t.
      int value;
    };

    //! Validate a command line option in the Boost.ProgramOptions
    //! framework.
    IML_DLLEXPORT
      void validate
        (::boost::any&, std::vector<std::string> const&, NetdevID*, int);
  }
}

#include <iml/gaspi/NetdevID.ipp>
