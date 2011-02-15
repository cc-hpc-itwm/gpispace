#ifndef GPI_SPACE_PC_TYPE_TIMESTAMP_HPP
#define GPI_SPACE_PC_TYPE_TIMESTAMP_HPP 1

#include <time.h>
#include <sys/time.h>

// serialization
#include <boost/serialization/nvp.hpp>

#include <gpi-space/pc/type/typedefs.hpp>

namespace boost
{
  namespace serialization
  {
    template <typename Archive>
    void serialize (Archive & ar, timeval & tv, const unsigned int /*version*/)
    {
      ar & tv.tv_sec;
      ar & tv.tv_usec;
    }
  }
}

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      struct time_stamp_t
      {
        enum touch_type
          {
            TOUCH_ALL,
            TOUCH_ACCESSED,
            TOUCH_MODIFIED,
          };

        gpi::pc::type::time_t created;
        gpi::pc::type::time_t accessed;
        gpi::pc::type::time_t modified;

        time_stamp_t ()
          : created (now())
          , accessed (created)
          , modified (created)
        {}

        void touch (const touch_type t = TOUCH_ALL)
        {
          gpi::pc::type::time_t n (now());

          if (t == TOUCH_ALL || t == TOUCH_ACCESSED)
          {
            accessed = n;
          }
          if (t == TOUCH_ALL || t == TOUCH_MODIFIED)
          {
            modified = n;
          }
        }

        static gpi::pc::type::time_t now ()
        {
          return time(NULL);
        }

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( created );
            ar & BOOST_SERIALIZATION_NVP( accessed );
            ar & BOOST_SERIALIZATION_NVP( modified );
          }
      };
    }
  }
}

#endif
