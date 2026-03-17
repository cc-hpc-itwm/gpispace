// Copyright (C) 2011,2014-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <list>
#include <map>
#include <ostream>

#include <variant>

#include <gspc/util/serialization/exception.hpp>

#include <gspc/iml/MemoryLocation.hpp>
#include <gspc/iml/MemoryRegion.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/SegmentHandle.hpp>
#include <gspc/iml/vmem/gaspi/pc/type/handle_descriptor.hpp>

#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/variant.hpp>




      namespace gpi::pc::proto::memory
      {
        struct alloc_t
        {
          gspc::iml::SegmentHandle segment;
          gspc::iml::MemorySize size;

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
          gspc::iml::AllocationHandle handle;

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
          gspc::iml::AllocationHandle handle;

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
          gspc::iml::MemoryLocation dst;
          gspc::iml::MemoryLocation src;
          gspc::iml::MemorySize size;

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
                (gspc::util::serialization::exception::serialize (_exception));
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
              _exception = gspc::util::serialization::exception::deserialize
                (exception);
            }
          }
        };

        struct info_t
        {
          gspc::iml::AllocationHandle handle;
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
          get_transfer_costs_t (std::list<gspc::iml::MemoryRegion> transfers_)
            : transfers (std::move (transfers_))
          {}

          std::list<gspc::iml::MemoryRegion> transfers;
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

        using message_t = std::variant< memory::alloc_t
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
