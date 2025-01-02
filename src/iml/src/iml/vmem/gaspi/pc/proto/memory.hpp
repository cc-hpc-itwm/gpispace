// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <list>
#include <map>
#include <ostream>

#include <boost/variant.hpp>

#include <util-generic/serialization/exception.hpp>

#include <iml/MemoryLocation.hpp>
#include <iml/MemoryRegion.hpp>
#include <iml/MemorySize.hpp>
#include <iml/SegmentHandle.hpp>
#include <iml/vmem/gaspi/pc/type/handle_descriptor.hpp>

#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/variant.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace memory
      {
        struct alloc_t
        {
          iml::SegmentHandle segment;
          iml::MemorySize size;

        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & segment;
            ar & size;
          }
        };

        struct alloc_reply_t
        {
          iml::AllocationHandle handle;

        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & handle;
          }
        };

        struct free_t
        {
          iml::AllocationHandle handle;

        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & handle;
          }
        };

        struct memcpy_t
        {
          iml::MemoryLocation dst;
          iml::MemoryLocation src;
          iml::MemorySize size;

        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & dst;
            ar & src;
            ar & size;
          }
        };

        struct memcpy_reply_t
        {
          gpi::pc::type::memcpy_id_t memcpy_id;

        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & memcpy_id;
          }
        };

        struct wait_t
        {
          gpi::pc::type::memcpy_id_t memcpy_id;

        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & memcpy_id;
          }
        };

        struct wait_reply_t
        {
          wait_reply_t (std::exception_ptr exception = nullptr)
            : _exception (exception)
          {}

          void get() const
          {
            if (_exception)
            {
              std::rethrow_exception (_exception);
            }
          }

        private:
          std::exception_ptr _exception;

          friend class ::boost::serialization::access;
          template<class Archive>
            void serialize (Archive & ar, unsigned int const version)
          {
            ::boost::serialization::split_member (ar, *this, version);
          }

          template<typename Archive>
            void save (Archive& ar, unsigned int const) const
          {
            ar << !!_exception;
            if (_exception)
            {
              std::string const exception
                (fhg::util::serialization::exception::serialize (_exception));
              ar << exception;
            }
          }
          template<typename Archive>
            void load (Archive& ar, unsigned int const)
          {
            bool has_exception;
            ar >> has_exception;
            if (has_exception)
            {
              std::string exception;
              ar >> exception;
              _exception = fhg::util::serialization::exception::deserialize
                (exception);
            }
          }
        };

        struct info_t
        {
          iml::AllocationHandle handle;
        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & handle;
          }
        };

        struct info_reply_t
        {
          gpi::pc::type::handle::descriptor_t descriptor;
        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & descriptor;
          }
        };

        struct get_transfer_costs_t
        {
          get_transfer_costs_t () {}
          get_transfer_costs_t (std::list<iml::MemoryRegion> transfers_)
            : transfers (std::move (transfers_))
          {}

          std::list<iml::MemoryRegion> transfers;
        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & transfers;
          }
        };

        struct transfer_costs_t
        {
          transfer_costs_t () {}
          transfer_costs_t (std::unordered_map<std::string, double> costs_)
            : costs (std::move (costs_))
          {}

          std::unordered_map<std::string, double> costs;
        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & costs;
          }
        };

        using message_t = ::boost::variant< memory::alloc_t
                                          , memory::alloc_reply_t
                                          , memory::free_t
                                          , memory::memcpy_t
                                          , memory::memcpy_reply_t
                                          , memory::wait_t
                                          , memory::wait_reply_t
                                          , memory::info_t
                                          , memory::info_reply_t
                                          , memory::get_transfer_costs_t
                                          , memory::transfer_costs_t
                                          >;
      }
    }
  }
}
