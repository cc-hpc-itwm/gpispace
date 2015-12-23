#pragma once

#include <boost/variant.hpp>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/segment_descriptor.hpp>

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
          gpi::pc::type::size_t  size;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( size );
          }
        };

        struct register_reply_t
        {
          gpi::pc::type::segment_id_t id;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
          }
        };

        struct unregister_t
        {
          gpi::pc::type::segment_id_t id;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
          }
        };

        // replies with add_reply_t
        struct add_memory_t
        {
          add_memory_t ()
            : url ("")
          {}

          add_memory_t (std::string const & a_url)
            : url (a_url)
          {}

          std::string            url;
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP (url);
          }
        };

        struct add_reply_t
        {
          //! \note serialization only
          add_reply_t() = default;
          add_reply_t (std::exception_ptr exception)
            : _exception (std::move (exception))
          {}
          add_reply_t (type::segment_id_t segment)
            : _exception (nullptr)
            , _segment (segment)
          {}

          type::segment_id_t get() const
          {
            if (_exception)
            {
              std::rethrow_exception (_exception);
            }

            return _segment;
          }

        private:
          std::exception_ptr _exception;
          type::segment_id_t _segment;

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
                ( fhg::util::serialization::exception::serialize
                    ( _exception
                    , fhg::util::serialization::exception::serialization_functions()
                    , fhg::util::serialization::exception::aggregated_serialization_functions()
                    )
                );
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
                ( exception
                , fhg::util::serialization::exception::serialization_functions()
                , fhg::util::serialization::exception::aggregated_serialization_functions()
                );
            }
          }
        };

        // replies with error message
        struct del_memory_t
        {
          del_memory_t ()
            : id (0)
          {}

          explicit
          del_memory_t (gpi::pc::type::segment_id_t seg_id)
            : id (seg_id)
          {}

          gpi::pc::type::segment_id_t id;
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
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
