#pragma once

#include <list>
#include <map>
#include <ostream>

#include <boost/variant.hpp>

#include <util-generic/serialization/exception.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>
#include <iml/vmem/gaspi/pc/type/handle_descriptor.hpp>
#include <iml/vmem/gaspi/pc/type/memory_location.hpp>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
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
          gpi::pc::type::segment_id_t segment;
          gpi::pc::type::size_t size;
          gpi::pc::type::name_t name;
          gpi::pc::type::flags_t flags;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( segment );
            ar & BOOST_SERIALIZATION_NVP( size );
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( flags );
          }
        };

        struct alloc_reply_t
        {
          gpi::pc::type::handle_id_t handle;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( handle );
          }
        };

        struct free_t
        {
          gpi::pc::type::handle_id_t handle;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( handle );
          }
        };

        struct memcpy_t
        {
          gpi::pc::type::memory_location_t dst;
          gpi::pc::type::memory_location_t src;
          gpi::pc::type::size_t size;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( dst );
            ar & BOOST_SERIALIZATION_NVP( src );
            ar & BOOST_SERIALIZATION_NVP( size );
          }
        };

        struct memcpy_reply_t
        {
          gpi::pc::type::memcpy_id_t memcpy_id;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( memcpy_id );
          }
        };

        struct wait_t
        {
          gpi::pc::type::memcpy_id_t memcpy_id;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( memcpy_id );
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

          friend class boost::serialization::access;
          template<class Archive>
            void serialize (Archive & ar, unsigned int const version)
          {
            boost::serialization::split_member (ar, *this, version);
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
          gpi::pc::type::handle_id_t handle;
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( handle );
          }
        };

        struct info_reply_t
        {
          gpi::pc::type::handle::descriptor_t descriptor;
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( descriptor );
          }
        };

        struct get_transfer_costs_t
        {
          get_transfer_costs_t () {}
          get_transfer_costs_t (std::list<gpi::pc::type::memory_region_t> const& transfers_)
            : transfers (transfers_)
          {}

          std::list<gpi::pc::type::memory_region_t> transfers;
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP (transfers);
          }
        };

        struct transfer_costs_t
        {
          transfer_costs_t () {}
          transfer_costs_t (std::map<std::string, double> const& costs_)
            : costs (costs_)
          {}

          std::map<std::string, double> costs;
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP (costs);
          }
        };

        typedef boost::variant<
          memory::alloc_t
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
          > message_t;
      }
    }
  }
}
