// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_STATE_HPP
#define _XML_PARSE_STATE_HPP

#include <we/type/signature.hpp>

#include <iostream>

#include <list>
#include <deque>

#include <stdexcept>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

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
      namespace po = boost::program_options;

      typedef std::vector<fs::path> search_path_type;
      typedef std::vector<fs::path> in_progress_type;

      // ******************************************************************* //

      struct type
      {
      private:
        int _level;
        search_path_type _search_path;
        in_progress_type _in_progress;
        bool _Werror;
        bool _Woverwrite_function_name;
        bool _Wshadow;
        bool _Wdefault_construction;
        bool _Wunused_field;
        bool _Wport_not_connected;

        template<typename W>
        void generic_warn (const W & w, const bool & active) const
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

        fs::path expand (const std::string & file) const
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
          , _Wshadow (true)
          , _Wdefault_construction (true)
          , _Wunused_field (true)
          , _Wport_not_connected (true)
        {}

        int & level (void) { return _level; }
        const search_path_type & search_path (void) const
        {
          return _search_path;
        }
        
        const bool & Werror (void) const { return _Werror; }
        const bool & Woverwrite_function_name (void) const { return _Woverwrite_function_name; }
        const bool & Wshadow (void) const { return _Wshadow; }
        const bool & Wdefault_construction (void) const { return _Wdefault_construction; }
        const bool & Wunused_field (void) const { return _Wunused_field; }
        const bool & Wport_not_connected (void) const { return _Wport_not_connected; }

        // ***************************************************************** //

        fs::path file_in_progress (void) const
        {
          return (_in_progress.empty()) 
            ? fs::path("<stdin>") 
            : _in_progress.back()
            ;
        }

        // ***************************************************************** //

        void warn (const warning::overwrite_function_name & w) const
        {
          generic_warn (w, _Woverwrite_function_name);
        }

        template<typename T>
        void warn (const warning::struct_shadowed<T> & w) const
        {
          generic_warn (w, _Wshadow);
        }

        void warn (const warning::default_construction & w) const
        {
          generic_warn (w, _Wdefault_construction);
        }

        void warn (const warning::unused_field & w) const
        {
          generic_warn (w, _Wunused_field);
        }

        void warn (const warning::port_not_connected & w) const
        {
          generic_warn (w, _Wport_not_connected);
        }

        // ***************************************************************** //

        template<typename T>
        T generic_include ( T (*parse)(std::istream &, type &)
                          , const std::string & file
                          )
        {
          const fs::path path (expand (file));

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

        // ***************************************************************** //

        void add_options (po::options_description & desc)
        {
          desc.add_options ()
            ( "search-path"
            , po::value<search_path_type>(&_search_path)
            , "search path"
            )
            ( "Werror"
            , po::value<bool>(&_Werror)->default_value(_Werror)
            , "cast warnings to errors"
            )
            ( "Woverwrite_function_name"
            , po::value<bool>(&_Woverwrite_function_name)
                              ->default_value(_Woverwrite_function_name)
            , "warn when overwriting a function name"
            )
            ( "Wshadow"
            , po::value<bool>(&_Wshadow)->default_value(_Wshadow)
            , "warn when shadowing a struct definition"
            )
            ( "Wdefault_construction"
            , po::value<bool>(&_Wdefault_construction)
                              ->default_value(_Wdefault_construction)
            , "warn when default construct (part of) tokens"
            )
            ( "Wunused_field"
            , po::value<bool>(&_Wunused_field)
                              ->default_value(_Wunused_field)
            , "warn when given fields in tokens are unused"
            )
            ;
        }
      };
    }
  }
}

#endif
