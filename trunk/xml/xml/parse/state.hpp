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
#include <xml/parse/util/read_bool.hpp>

// ************************************************************************* //

namespace xml
{
  namespace parse
  {
    namespace state
    {
      namespace fs = boost::filesystem;
      namespace po = boost::program_options;
      namespace property = we::type::property;

      typedef std::vector<fs::path> search_path_type;
      typedef std::vector<fs::path> in_progress_type;

      // ******************************************************************* //

      struct type
      {
      private:
        int _level;
        search_path_type _search_path;
        in_progress_type _in_progress;
        property::path_type _prop_path;
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
        bool _Wtype_get_duplicate;

        bool _print_internal_structures;
        bool _no_inline;

        std::string _Osearch_path;
        std::string _Oignore_properties;
        std::string _OWerror;
        std::string _OWall;
        std::string _OWoverwrite_function_name_as;
        std::string _OWoverwrite_template_name_as;
        std::string _OWshadow;
        std::string _OWdefault_construction;
        std::string _OWunused_field;
        std::string _OWport_not_connected;
        std::string _OWunexpected_element;
        std::string _OWoverwrite_function_name_trans;
        std::string _OWproperty_overwritten;
        std::string _OWtype_map_duplicate;
        std::string _OWtype_get_duplicate;
        
        std::string _Oprint_internal_structures;
        std::string _Ono_inline;

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
          , _Wtype_get_duplicate(true)
          , _print_internal_structures(false)
          , _no_inline(false)

          , _Osearch_path ("search-path")
          , _Oignore_properties ("ignore-properties")
          , _OWerror ("Werror")
          , _OWall ("all")
          , _OWoverwrite_function_name_as ("Woverwrite-function-name-as")
          , _OWoverwrite_template_name_as ("Woverwrite-template-name-as")
          , _OWshadow ("shadow")
          , _OWdefault_construction ("Wdefault-construction")
          , _OWunused_field ("Wunused-field")
          , _OWport_not_connected ("Wport-not-connected")
          , _OWunexpected_element ("Wunexpected-element")
          , _OWoverwrite_function_name_trans ("Woverwrite-function-name-trans")
          , _OWproperty_overwritten ("Wproperty-overwritten")
          , _OWtype_map_duplicate ("Wtype-map-duplicate")
          , _OWtype_get_duplicate ("Wtype-get-duplicate")
          , _Oprint_internal_structures ("print-internal-structures")
          , _Ono_inline ("no-inline")
        {}

        int & level (void) { return _level; }
        const search_path_type & search_path (void) const
        {
          return _search_path;
        }

        // ***************************************************************** //

        property::path_type & prop_path (void)
        {
          return _prop_path;
        }

        // ***************************************************************** //

        bool property ( const property::path_type & path
                      , const property::value_type & value
                      )
        {
          if (path.size() != 2 || path[0] != "pnetc")
            {
              return false;
            }

#define GET_PROP(x)                       \
          else if (path[1] == _O ## x)    \
            {                             \
              _ ## x = read_bool (value); \
            }

          if (path[1] == _Osearch_path)
            {
              _search_path.push_back (value);
            }

          GET_PROP (ignore_properties)
          GET_PROP (Werror)
          GET_PROP (Wall)
          GET_PROP (Woverwrite_function_name_as)
          GET_PROP (Woverwrite_template_name_as)
          GET_PROP (Wshadow)
          GET_PROP (Wdefault_construction)
          GET_PROP (Wunused_field)
          GET_PROP (Wport_not_connected)
          GET_PROP (Wunexpected_element)
          GET_PROP (Woverwrite_function_name_trans)
          GET_PROP (Wproperty_overwritten)
          GET_PROP (Wtype_map_duplicate)
          GET_PROP (Wtype_get_duplicate)
          GET_PROP (print_internal_structures)
          GET_PROP (no_inline)

#undef GET_PROP

          return true;
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

#define ACCESS(x)                                       \
        const bool & x (void) const { return _ ## x; }  \
        bool & x (void){ return _ ## x; }

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
        ACCESS(Wtype_get_duplicate)
        
        ACCESS(print_internal_structures)
        ACCESS(no_inline)

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

        void warn (const warning::type_get_duplicate & w) const
        {
          generic_warn (w, _Wtype_get_duplicate);
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
            ( _Osearch_path.c_str()
            , po::value<search_path_type>(&_search_path)
            , "search path"
            )
            ( _Oignore_properties.c_str()
            , VAL(ignore_properties)
            , "when set to true, no properties are parsed"
            )
            ( _OWerror.c_str()
            , VAL(Werror)
            , "cast warnings to errors"
            )
            ( _OWall.c_str()
            , VAL(Wall)
            , "turn on all warnings"
            )
            ( _OWoverwrite_function_name_as.c_str()
            , VAL(Woverwrite_function_name_as)
            , "warn when overwriting a function name by 'as'"
            )
            ( _OWoverwrite_template_name_as.c_str()
            , VAL(Woverwrite_template_name_as)
            , "warn when overwriting a template name by 'as'"
            )
            ( _OWshadow.c_str()
            , VAL(Wshadow)
            , "warn when shadowing a struct definition"
            )
            ( _OWdefault_construction.c_str()
            , VAL(Wdefault_construction)
            , "warn when default construct (part of) tokens"
            )
            ( _OWunused_field.c_str()
            , VAL(Wunused_field)
            , "warn when given fields in tokens are unused"
            )
            ( _OWport_not_connected.c_str()
            , VAL(Wport_not_connected)
            , "warn when port are not connected"
            )
            ( _OWunexpected_element.c_str()
            , VAL(Wunexpected_element)
            , "warn when unexpected elements occur"
            )
            ( _OWoverwrite_function_name_trans.c_str()
            , VAL(Woverwrite_function_name_trans)
            , "warn when overwriting a function name with a transition name"
            )
            ( _OWproperty_overwritten.c_str()
            , VAL(Wproperty_overwritten)
            , "warn when overwriting a property"
            )
            ( _OWtype_map_duplicate.c_str()
            , VAL(Wtype_map_duplicate)
            , "warn about duplicate type maps"
            )
            ( _OWtype_get_duplicate.c_str()
            , VAL(Wtype_get_duplicate)
            , "warn about duplicate type gets"
            )
            ( _Oprint_internal_structures.c_str()
            , VAL(print_internal_structures)
            , "if set the parser dumps the internal structures right before synthesize"
            )
            ( _Ono_inline.c_str()
            , VAL(no_inline)
            , "if set, ignore the keyword inline"
            )
            ;
#undef VAL
        }
      };
    }
  }
}

#endif
