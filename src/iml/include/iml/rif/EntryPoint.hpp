// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/detail/dllexport.hpp>

#include <cstddef>
#include <ostream>
#include <string>

namespace iml
{
  namespace rif
  {
    //! Information on how to contact a running RIF server, obtained
    //! from \c Rifds.
    struct IML_DLLEXPORT EntryPoint
    {
      std::string hostname;
      unsigned short port;
      pid_t pid;

      //! Create a RIF entry point from the string \a input.
      EntryPoint (std::string const& input);
      //! Print \a entry_point to \a os in a form that can be used to
      //! construct an \c EntryPoint from.
      IML_DLLEXPORT friend std::ostream& operator<<
        (std::ostream& os, EntryPoint const& entry_point);

      //! \note For serialization only.
      EntryPoint() = default;

      //! Serialize using Boost.Serialization.
      template<typename BoostArchive>
        void serialize (BoostArchive& archive, unsigned int);

      //! Create a RIF entry point denoting the process \a pid_
      //! running on node \a hostname_, reachable at port \a port_.
      EntryPoint
        (std::string const& hostname_, unsigned short port_, pid_t pid_);

      friend bool operator== (EntryPoint const&, EntryPoint const&);
    };
  }
}

namespace std
{
  template<>
    struct IML_DLLEXPORT hash<iml::rif::EntryPoint>
  {
    std::size_t operator() (iml::rif::EntryPoint const&) const;
  };
}

#include <iml/rif/EntryPoint.ipp>
