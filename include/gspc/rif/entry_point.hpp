// Copyright (C) 2015,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/hash/combined_hash.hpp>
#include <gspc/util/ostream/modifier.hpp>

#include <string>
#include <tuple>

#include <unistd.h>


  namespace gspc::rif
  {
    struct entry_point : public gspc::util::ostream::modifier
    {
      std::string hostname;
      unsigned short port;
      pid_t pid;

      entry_point (std::string const& hostname, unsigned short port, pid_t pid);

      //! parse input
      entry_point (std::string const& input);

      std::ostream& operator() (std::ostream&) const override;

      bool operator== (entry_point const&) const;

      //! \note Serialization only.
      entry_point();
      template<typename Archive> void serialize (Archive&, unsigned int);
    };
  }


FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( gspc::rif::entry_point
  , ep
  , ep.hostname
  , ep.port
  , ep.pid
  )

#include <gspc/rif/entry_point.ipp>
