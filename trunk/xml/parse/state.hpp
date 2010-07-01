// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_STATE_HPP
#define _XML_PARSE_STATE_HPP

#include <we/type/signature.hpp>

#include <iostream>

#include <list>

#include <stdexcept>

#include <boost/filesystem.hpp>

// ************************************************************************* //

namespace xml
{
  namespace parse
  {
    namespace state
    {
      namespace fs = boost::filesystem;

      typedef std::vector<fs::path> search_path_type;

      struct type
      {
      private:
        int _level;
        search_path_type _search_path;

      public:
        type (void)
          : _level (0)
          , _search_path ()
        {}

        int & level (void) { return _level; }
        search_path_type & search_path (void) { return _search_path; }

        fs::path expand (const std::string & file)
        {
          for ( search_path_type::const_iterator dir (_search_path.begin())
              ; dir != _search_path.end()
              ; ++dir
              )
            {
              if (! fs::exists (*dir))
                {
                  continue;
                }

              fs::path path (*dir / file);

              if (fs::exists (path))
                {
                  return path;
                }
            }

          throw std::runtime_error ("could not find file: " + file);
        }
      };
    }
  }
}

#endif
