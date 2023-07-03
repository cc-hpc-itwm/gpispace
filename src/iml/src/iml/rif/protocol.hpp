// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-rpc/function_description.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/serialization/exception.hpp>
#include <util-generic/serialization/std/chrono.hpp>

#include <iml/gaspi/NetdevID.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

#include <chrono>
#include <exception>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <unistd.h>

namespace fhg
{
  namespace iml
  {
    namespace gaspi = ::iml::gaspi;
    namespace rif
    {
      namespace protocol
      {
        FHG_RPC_FUNCTION_DESCRIPTION
          ( kill
          , std::unordered_map<pid_t, std::exception_ptr> (std::vector<pid_t>)
          );

        FHG_RPC_FUNCTION_DESCRIPTION
          ( start_vmem
          , pid_t ( ::boost::filesystem::path socket
                  , unsigned short gaspi_port
                  , std::chrono::seconds proc_init_timeout
                  , std::vector<std::string> nodes
                  , std::size_t rank
                  , gaspi::NetdevID netdev_id
                  )
          );
      }
    }
  }
}

namespace boost
{
  namespace serialization
  {
    template<class Archive>
      inline void save
        ( Archive& ar
        , std::unordered_map<pid_t, std::exception_ptr> const& t
        , unsigned int
        )
    {
      std::size_t const size (t.size());
      ar << size;
      for (auto const& x : t)
      {
        std::string const exception_string
          (fhg::util::serialization::exception::serialize (x.second));
        ar << x.first;
        ar << exception_string;
      }
    }

    template<class Archive>
      inline void load
        ( Archive& ar
        , std::unordered_map<pid_t, std::exception_ptr>& t
        , unsigned int
        )
    {
      std::size_t size;
      ar >> size;
      while (size --> 0)
      {
        pid_t pid;
        std::string exception_string;
        ar >> pid;
        ar >> exception_string;
        t.emplace
          ( pid
          , fhg::util::serialization::exception::deserialize (exception_string)
          );
      }
    }

    template<class Archive, class Compare, class Allocator>
      inline void serialize
        ( Archive& ar
        , std::unordered_map<pid_t, std::exception_ptr>& t
        , unsigned int file_version
        )
    {
      ::boost::serialization::split_free (ar, t, file_version);
    }
  }
}
