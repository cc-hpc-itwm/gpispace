#ifndef WE_MGMT_TYPE_FLAGS_HPP
#define WE_MGMT_TYPE_FLAGS_HPP 1

#include <boost/serialization/nvp.hpp>

#include <iosfwd>

namespace we
{
  namespace mgmt
  {
    namespace flags
    {
      struct flags_t
      {
        bool suspended;
        bool cancelling;
        bool cancelled;
        bool failed;
        bool finished;

        flags_t ()
          : suspended(false)
          , cancelling(false)
          , cancelled(false)
          , failed(false)
          , finished(false)
        { }

      private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize (Archive & ar, const unsigned int)
        {
          ar & BOOST_SERIALIZATION_NVP(suspended);
          ar & BOOST_SERIALIZATION_NVP(cancelling);
          ar & BOOST_SERIALIZATION_NVP(cancelled);
          ar & BOOST_SERIALIZATION_NVP(failed);
          ar & BOOST_SERIALIZATION_NVP(finished);
        }
      };

      std::ostream& operator<< (std::ostream&, const flags_t&);

      template <typename T>
      struct flag_traits {};

      template <>
      struct flag_traits<flags_t>
      {
        inline static bool is_alive ( const flags_t & f )
        {
          return ( f.suspended || f.cancelling || f.cancelled || f.failed || f.finished ) == false;
        }

        inline static bool is_suspended ( const flags_t & f )
        {
          return f.suspended;
        }

        inline static void set_suspended ( flags_t & f, const bool val )
        {
          f.suspended = val;
        }

        inline static bool is_cancelling ( const flags_t & f )
        {
          return f.cancelling;
        }

        inline static void set_cancelling ( flags_t & f, const bool val )
        {
          f.cancelling = val;
        }

        inline static bool is_cancelled ( const flags_t & f )
        {
          return f.cancelled;
        }

        inline static void set_cancelled ( flags_t & f, const bool val )
        {
          f.cancelled = val;
        }

        inline static bool is_failed ( const flags_t & f )
        {
          return f.failed;
        }

        inline static void set_failed ( flags_t & f, const bool val )
        {
          f.failed = val;
        }

        inline static bool is_finished ( const flags_t & f )
        {
          return f.finished;
        }

        inline static void set_finished ( flags_t & f, const bool val )
        {
          f.finished = val;
        }
      };
    }
  }
}


#endif
