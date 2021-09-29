// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <boost/variant.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>

#include <iml/MemorySize.hpp>
#include <iml/SegmentDescription.hpp>
#include <iml/SegmentHandle.hpp>
#include <iml/SharedMemoryAllocationHandle.hpp>
#include <iml/vmem/gaspi/pc/type/impl_types.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace segment
      {
        struct register_t
        {
          gpi::pc::type::name_t  name;
          iml::MemorySize  size;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( size );
          }
        };

        struct register_reply_t
        {
          iml::SharedMemoryAllocationHandle handle;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( handle );
          }
        };

        struct unregister_t
        {
          iml::SharedMemoryAllocationHandle handle;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( handle );
          }
        };

        // replies with add_reply_t
        struct add_memory_t
        {
          iml::SegmentDescription description;
          unsigned long total_size;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & description;
            ar & total_size;
          }
        };

        struct add_reply_t
        {
          //! \note serialization only
          add_reply_t() = default;
          add_reply_t (std::exception_ptr exception)
            : _exception (std::move (exception))
            , _segment()
          {}
          add_reply_t (iml::SegmentHandle segment)
            : _exception (nullptr)
            , _segment (segment)
          {}

          iml::SegmentHandle get() const
          {
            if (_exception)
            {
              std::rethrow_exception (_exception);
            }

            return _segment;
          }

        private:
          std::exception_ptr _exception;
          iml::SegmentHandle _segment;

          friend class boost::serialization::access;
          template<class Archive>
            void serialize (Archive & ar, unsigned int const version)
          {
            boost::serialization::split_member (ar, *this, version);
          }

          template<typename Archive>
            void save (Archive& ar, unsigned int const) const
          {
            ar << _segment;
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
            ar >> _segment;
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

        // replies with error message
        struct del_memory_t
        {
          del_memory_t () = default;

          explicit
          del_memory_t (iml::SegmentHandle seg_id)
            : id (seg_id)
          {}

          iml::SegmentHandle id;
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
          }
        };

        typedef boost::variant< segment::register_t
                              , segment::register_reply_t
                              , segment::unregister_t
                              , segment::add_memory_t
                              , segment::add_reply_t
                              , segment::del_memory_t
                              > message_t;
      }
    }
  }
}
