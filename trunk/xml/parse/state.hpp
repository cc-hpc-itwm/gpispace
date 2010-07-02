// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_STATE_HPP
#define _XML_PARSE_STATE_HPP

#include <we/type/signature.hpp>

#include <iostream>

#include <list>

#include <stdexcept>

#include <boost/filesystem.hpp>

#include <parse/warning.hpp>

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
        bool _Werror;
        bool _Woverwrite_function_name;

        template<typename W>
        void generic_warn (const W & w, const bool & active)
        {
          if (active)
            {
              if (_Werror)
                {
                  throw w;
                }
              else
                {
                  std::cerr << w.what() << std::endl;
                }
            }
        };

      public:
        type (void)
          : _level (0)
          , _search_path ()
          , _Werror (false)
          , _Woverwrite_function_name (true)
        {}

        int & level (void) { return _level; }
        search_path_type & search_path (void) { return _search_path; }

        void warn (const warning::overwrite_function_name & w)
        {
          generic_warn (w, _Woverwrite_function_name);
        }

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
