// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_ID_HPP
#define FUSE_ID_HPP 1

#include <string>

namespace gpi_fuse
{
  namespace id
  {
    typedef unsigned long id_t;

    id_t parse ( std::string::const_iterator & pos
               , const std::string::const_iterator & end
               )
    {
      id_t id (0);

      while (pos != end && isdigit (*pos))
        {
          id *= 10;
          id += *pos - '0';
          ++pos;
        }

      return id;
    }
  } // namespace id
} // namespace gpi_fuse

#endif
