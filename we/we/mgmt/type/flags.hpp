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

      bool is_alive (const flags_t&);

#define SIG(_name)                         \
      bool is_ ## _name (const flags_t&);  \
      void set_ ## _name (flags_t&, bool)

      SIG (suspended);
      SIG (cancelling);
      SIG (cancelled);
      SIG (failed);
      SIG (finished);

#undef SIG
    }
  }
}


#endif
