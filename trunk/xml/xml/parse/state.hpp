// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_STATE_HPP
#define _XML_PARSE_STATE_HPP

#include <we/type/signature.hpp>
#include <we/type/property.hpp>

#include <iostream>

#include <list>
#include <deque>

#include <stdexcept>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <xml/parse/warning.hpp>
#include <xml/parse/error.hpp>

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
        we::type::property::path_type _prop_path;
        bool _ignore_properties;
        bool _Werror;
        bool _Wall;
        bool _Woverwrite_function_name_as;
        bool _Woverwrite_template_name_as;
        bool _Wshadow;
        bool _Wdefault_construction;
        bool _Wunused_field;
        bool _Wport_not_connected;
        bool _Wunexpected_element;
        bool _Woverwrite_function_name_trans;
        bool _Wproperty_overwritten;
        bool _Wtype_map_duplicate;

        template<typename W>
        void generic_warn (const W & w, const bool & active) const
        {
          if (_Wall || active)
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
          , _ignore_properties (false)
          , _Werror (false)
          , _Wall (false)
          , _Woverwrite_function_name_as (true)
          , _Woverwrite_template_name_as (true)
          , _Wshadow (true)
          , _Wdefault_construction (true)
          , _Wunused_field (true)
          , _Wport_not_connected (true)
          , _Wunexpected_element (true)
          , _Woverwrite_function_name_trans (false)
          , _Wproperty_overwritten (true)
          , _Wtype_map_duplicate(true)
        {}

        int & level (void) { return _level; }
        const search_path_type & search_path (void) const
        {
          return _search_path;
        }

        // ***************************************************************** //

        we::type::property::path_type & prop_path (void)
        {
          return _prop_path;
        }

        // ***************************************************************** //

        fs::path file_in_progress (void) const
        {
          return (_in_progress.empty()) 
            ? fs::path("<stdin>") 
            : _in_progress.back()
            ;
        }

        // ***************************************************************** //

#define ACCESS(x) const bool & x (void) const { return _ ## x; }

        ACCESS(ignore_properties)

        ACCESS(Werror)
        ACCESS(Wall)
        ACCESS(Woverwrite_function_name_as)
        ACCESS(Woverwrite_template_name_as)
        ACCESS(Wshadow)
        ACCESS(Wdefault_construction)
        ACCESS(Wunused_field)
        ACCESS(Wport_not_connected)
        ACCESS(Wunexpected_element)
        ACCESS(Woverwrite_function_name_trans)
        ACCESS(Wproperty_overwritten)
        ACCESS(Wtype_map_duplicate)

#undef ACCESS

        // ***************************************************************** //

        template<typename T>
        void warn (const warning::struct_shadowed<T> & w) const
        {
          generic_warn (w, _Wshadow);
        }

        void warn (const warning::overwrite_function_name_as & w) const
        {
          generic_warn (w, _Woverwrite_function_name_as);
        }

        void warn (const warning::overwrite_template_name_as & w) const
        {
          generic_warn (w, _Woverwrite_template_name_as);
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

        void warn (const warning::unexpected_element & w) const
        {
          generic_warn (w, _Wunexpected_element);
        }

        void warn (const warning::overwrite_function_name_trans & w) const
        {
          generic_warn (w, _Woverwrite_function_name_trans);
        }

        void warn (const warning::property_overwritten & w) const
        {
          generic_warn (w, _Wproperty_overwritten);
        }

        void warn (const warning::type_map_duplicate & w) const
        {
          generic_warn (w, _Wtype_map_duplicate);
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
#define VAL(x) po::value<bool>(&_ ## x)->default_value (_ ## x)

          desc.add_options ()
            ( "search-path"
            , po::value<search_path_type>(&_search_path)
            , "search path"
            )
            ( "ignore-properties"
            , VAL(ignore_properties)
            , "when set to true, no properties are parsed"
            )
            ( "Werror"
            , VAL(Werror)
            , "cast warnings to errors"
            )
            ( "Wall"
            , VAL(Wall)
            , "turn on all warnings"
            )
            ( "Woverwrite_function_name_as"
            , VAL(Woverwrite_function_name_as)
            , "warn when overwriting a function name by 'as'"
            )
            ( "Woverwrite_template_name_as"
            , VAL(Woverwrite_template_name_as)
            , "warn when overwriting a template name by 'as'"
            )
            ( "Wshadow"
            , VAL(Wshadow)
            , "warn when shadowing a struct definition"
            )
            ( "Wdefault_construction"
            , VAL(Wdefault_construction)
            , "warn when default construct (part of) tokens"
            )
            ( "Wunused_field"
            , VAL(Wunused_field)
            , "warn when given fields in tokens are unused"
            )
            ( "Wunexpected_element"
            , VAL(Wunexpected_element)
            , "warn when unexpected elements occur"
            )
            ( "Woverwrite_function_name_trans"
            , VAL(Woverwrite_function_name_trans)
            , "warn when overwriting a function name with a transition name"
            )
            ( "Wproperty_overwritten"
            , VAL(Wproperty_overwritten)
            , "warn when overwriting a property"
            )
            ( "Wtype_map_duplicate"
            , VAL(Wtype_map_duplicate)
            , "warn about duplicate type maps"
            )
            ;
#undef VAL
        }
      };
    }
  }
}

#endif
