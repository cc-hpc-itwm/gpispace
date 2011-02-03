// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_SEGMENT_HPP
#define FUSE_SEGMENT_HPP 1

#include <string>
#include <sstream>
#include <list>

#include <stdexcept>

#include <id.hpp>

namespace gpi_fuse
{
  namespace segment
  {
    typedef id::id_t id_t;
    typedef std::list<id_t> id_list_t;

    static inline std::string root ()
    {
      static const std::string r ("/");

      return r;
    }
    static inline std::string global ()
    {
      static const std::string g ("global");

      return g;
    }
    static inline std::string local ()
    {
      static const std::string l ("local");

      return l;
    }
    static inline std::string shared ()
    {
      static const std::string l ("shared");

      return l;
    }
    static inline std::string proc ()
    {
      static const std::string p ("proc");

      return p;
    }

    static inline std::string string (const id::id_t id)
    {
      switch (id)
        {
        case 0: return global();
        case 1: return local();
        default:
          {
            std::ostringstream str;

            str << shared() << "/" << id;

            return str.str();
          }
        }

      throw std::runtime_error ("segment::string: STRANGE!");
    }
  } // namespace segment
} // namespace gpi_fuse

#endif
