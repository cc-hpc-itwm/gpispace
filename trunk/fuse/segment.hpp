// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_SEGMENT_HPP
#define FUSE_SEGMENT_HPP 1

#include <string>
#include <list>

#include <id.hpp>

namespace gpi_fuse
{
  namespace segment
  {
    typedef id::id_t id_t;
    typedef std::list<id_t> id_list_t;

    static std::string global ()
    {
      static const std::string g ("global");

      return g;
    }

    static std::string local ()
    {
      static const std::string l ("local");

      return l;
    }

    static std::string shared ()
    {
      static const std::string l ("shared");

      return l;
    }

    static std::string proc ()
    {
      static const std::string p ("proc");

      return p;
    }
  } // namespace segment
} // namespace gpi_fuse

#endif
