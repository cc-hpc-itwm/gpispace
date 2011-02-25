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
        static const int TOUCH_ACCESSED = 0x01;
        static const int TOUCH_MODIFIED = 0x02;
        static const int TOUCH_ALL = (TOUCH_ACCESSED | TOUCH_MODIFIED);

        gpi::pc::type::time_t created;
        gpi::pc::type::time_t accessed;
        gpi::pc::type::time_t modified;

        time_stamp_t ()
          : created (now())
          , accessed (created)
          , modified (created)
        {}

        void touch ()
        {
          touch (TOUCH_ACCESSED | TOUCH_MODIFIED);
        }

        void touch (const int what)
        {
          touch (what, 0);
        }

        void touch (const int what, const gpi::pc::type::time_t tstamp)
        {
          const gpi::pc::type::time_t n ( (tstamp != 0) ? tstamp : now());

          if (what & TOUCH_ACCESSED)
          {
            accessed = n;
          }

          if (what & TOUCH_MODIFIED)
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
