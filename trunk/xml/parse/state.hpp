// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_STATE_HPP
#define _XML_PARSE_STATE_HPP

#include <we/type/signature.hpp>

#include <iostream>

#include <list>

#include <stdexcept>

#include <boost/filesystem.hpp>

#include <parse/warning.hpp>
#include <parse/error.hpp>

// ************************************************************************* //

namespace xml
{
  namespace parse
  {
    namespace state
    {
      namespace fs = boost::filesystem;

      typedef std::vector<fs::path> search_path_type;
      typedef std::vector<fs::path> in_progress_type;

      struct type
      {
      private:
        int _level;
        search_path_type _search_path;
        in_progress_type _in_progress;
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

        fs::path expand (const std::string & file)
        {
          fs::path direct (file);

          if (fs::exists (direct))
            {
              return direct;
            }

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

          throw error::file_not_found ("expand", file);
        }

      public:
        type (void)
          : _level (0)
          , _search_path ()
          , _in_progress ()
          , _Werror (false)
          , _Woverwrite_function_name (true)
        {}

        int & level (void) { return _level; }
        search_path_type & search_path (void) { return _search_path; }
        bool & Werror (void) { return _Werror; }
        bool & Woverwrite_function_name (void) { return _Woverwrite_function_name; }

        void warn (const warning::overwrite_function_name & w)
        {
          generic_warn (w, _Woverwrite_function_name);
        }

        template<typename T>
        T generic_include ( T (*parse)(std::istream &, type &)
                          , const std::string & file
                          )
        {
          const boost::filesystem::path path (expand (file));

          for ( in_progress_type::const_iterator pos (_in_progress.begin())
              ; pos != _in_progress.end()
              ; ++pos
              )
            {
              if (*pos == path)
                {
                  throw error::include_loop<in_progress_type::const_iterator>
                    ("generic_include", pos, _in_progress.end());
                }
            }

          _in_progress.push_back (path);

          std::ifstream stream (path.string().c_str());

          const T x (parse (stream, *this));

          _in_progress.pop_back ();

          return x;
        };
      };
    }
  }
}

#endif
