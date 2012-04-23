// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_STATE_HPP
#define _XML_PARSE_STATE_HPP

#include <we/type/signature.hpp>
#include <we/type/property.hpp>

#include <we/expr/eval/context.hpp>

#include <we/type/bits/transition/optimize.hpp>

#include <fstream>

#include <vector>
#include <list>

#include <stdexcept>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <xml/parse/warning.hpp>
#include <xml/parse/error.hpp>
#include <xml/parse/util/weparse.hpp>

#include <xml/parse/type/require.hpp>

#include <fhg/util/read_bool.hpp>
#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

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
      namespace optimize = we::type::optimize;

      using namespace warning;

      typedef std::vector<std::string> search_path_type;
      typedef std::vector<fs::path> in_progress_type;

      typedef std::vector<std::string> gen_param_type;

      // ******************************************************************* //

      struct key_value_t
      {
      private:
        const property::key_type _key;
        const property::value_type _value;

      public:
        key_value_t (const std::string & key, const std::string & value)
          : _key (key)
          , _value (value)
        {}

        const std::string & key () const { return _key; }
        const std::string & value () const { return _value; }
      };

      typedef std::list<key_value_t> key_values_t;

      // ******************************************************************* //

      struct type
      {
      public:
        typedef expr::eval::context context_t;

      private:
        ::xml::parse::type::requirements_type _requirements;
        search_path_type _search_path;
        gen_param_type _gen_ldflags;
        gen_param_type _gen_cxxflags;
        in_progress_type _in_progress;
        property::path_type _prop_path;
        context_t _context;
        key_values_t _key_values;
        optimize::options::type _options_optimize;
        bool _ignore_properties;
        bool _Werror;
        bool _Wall;
        bool _Woverwrite_function_name_as;
        bool _Woverwrite_template_name_as;
        bool _Wshadow_struct;
        bool _Wshadow_function;
        bool _Wdefault_construction;
        bool _Wunused_field;
        bool _Wport_not_connected;
        bool _Wunexpected_element;
        bool _Woverwrite_function_name_trans;
        bool _Woverwrite_function_internal_trans;
        bool _Wproperty_overwritten;
        bool _Wtype_map_duplicate;
        bool _Wtype_get_duplicate;
        bool _Woverwrite_context;
        bool _Windependent_place;
        bool _Windependent_transition;
        bool _Wconflicting_port_types;
        bool _Woverwrite_file;
        bool _Wbackup_file;
        bool _Wduplicate_external_function;
        bool _Wproperty_unknown;
        bool _Winline_many_output_ports;

        std::string _dump_xml_file;
        bool _no_inline;
        bool _synthesize_virtual_places;
        bool _force_overwrite_file;
        std::string _backup_extension;
        bool _do_file_backup;

        std::string _verbose_file;
        mutable bool _verbose_file_opened;
        mutable std::ofstream _verbose_stream;
        std::string _path_to_cpp;

        std::string _Osearch_path;
        std::string _Ogen_ldflags;
        std::string _Ogen_cxxflags;
        std::string _Oignore_properties;
        std::string _OWerror;
        std::string _OWall;
        std::string _OWoverwrite_function_name_as;
        std::string _OWoverwrite_template_name_as;
        std::string _OWshadow_struct;
        std::string _OWshadow_function;
        std::string _OWdefault_construction;
        std::string _OWunused_field;
        std::string _OWport_not_connected;
        std::string _OWunexpected_element;
        std::string _OWoverwrite_function_name_trans;
        std::string _OWoverwrite_function_internal_trans;
        std::string _OWproperty_overwritten;
        std::string _OWtype_map_duplicate;
        std::string _OWtype_get_duplicate;
        std::string _OWoverwrite_context;
        std::string _OWindependent_place;
        std::string _OWindependent_transition;
        std::string _OWconflicting_port_types;
        std::string _OWoverwrite_file;
        std::string _OWbackup_file;
        std::string _OWduplicate_external_function;
        std::string _OWproperty_unknown;
        std::string _OWinline_many_output_ports;

        std::string _Odump_xml_file;
        std::string _Ono_inline;
        std::string _Osynthesize_virtual_places;
        std::string _Oforce_overwrite_file;
        std::string _Obackup_extension;
        std::string _Odo_file_backup;

        std::string _Overbose_file;
        std::string _Opath_to_cpp;

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
          const fs::path absolute (file);

          if (fs::exists (absolute))
            {
              return absolute;
            }

          const fs::path from_actual_file
            (file_in_progress().parent_path() / file);

          if (fs::exists (from_actual_file))
            {
              return from_actual_file;
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

              const fs::path pre (*dir);
              const fs::path path (pre / file);

              if (fs::exists (path))
                {
                  return path;
                }
            }

          throw error::file_not_found ("expand", file);
        }

      public:
        type (void)
          : _search_path ()
          , _gen_ldflags ()
          , _gen_cxxflags ()
          , _in_progress ()
          , _ignore_properties (false)
          , _Werror (false)
          , _Wall (false)
          , _Woverwrite_function_name_as (false)
          , _Woverwrite_template_name_as (false)
          , _Wshadow_struct (true)
          , _Wshadow_function (true)
          , _Wdefault_construction (true)
          , _Wunused_field (true)
          , _Wport_not_connected (true)
          , _Wunexpected_element (true)
          , _Woverwrite_function_name_trans (false)
          , _Woverwrite_function_internal_trans (false)
          , _Wproperty_overwritten (true)
          , _Wtype_map_duplicate (true)
          , _Wtype_get_duplicate (true)
          , _Woverwrite_context (true)
          , _Windependent_place (true)
          , _Windependent_transition (true)
          , _Wconflicting_port_types (true)
          , _Woverwrite_file (true)
          , _Wbackup_file (true)
          , _Wduplicate_external_function (true)
          , _Wproperty_unknown (true)
          , _Winline_many_output_ports (true)

          , _dump_xml_file ("")
          , _no_inline (false)
          , _synthesize_virtual_places (false)
          , _force_overwrite_file (false)
          , _backup_extension ("~")
          , _do_file_backup (true)

          , _verbose_file ("")
          , _verbose_file_opened (false)
          , _path_to_cpp ("")

          , _Osearch_path ("search-path,I")
          , _Ogen_ldflags ("gen-ldflags")
          , _Ogen_cxxflags ("gen-cxxflags")
          , _Oignore_properties ("ignore-properties")
          , _OWerror ("Werror")
          , _OWall ("Wall")
          , _OWoverwrite_function_name_as ("Woverwrite-function-name-as")
          , _OWoverwrite_template_name_as ("Woverwrite-template-name-as")
          , _OWshadow_struct ("Wshadow-struct")
          , _OWshadow_function ("Wshadow-function")
          , _OWdefault_construction ("Wdefault-construction")
          , _OWunused_field ("Wunused-field")
          , _OWport_not_connected ("Wport-not-connected")
          , _OWunexpected_element ("Wunexpected-element")
          , _OWoverwrite_function_name_trans ("Woverwrite-function-name-trans")
          , _OWoverwrite_function_internal_trans ("Woverwrite-function-internal-trans")
          , _OWproperty_overwritten ("Wproperty-overwritten")
          , _OWtype_map_duplicate ("Wtype-map-duplicate")
          , _OWtype_get_duplicate ("Wtype-get-duplicate")
          , _OWoverwrite_context ("Woverwrite-context")
          , _OWindependent_place ("Windependent-place")
          , _OWindependent_transition ("Windependent-transition")
          , _OWconflicting_port_types ("Wconflicting-port-types")
          , _OWoverwrite_file ("Woverwrite-file")
          , _OWbackup_file ("Wbackup-file")
          , _OWduplicate_external_function ("Wduplicate-external-function")
          , _OWproperty_unknown ("Wproperty-unknown")
          , _OWinline_many_output_ports ("Winline-many-output-ports")

          , _Odump_xml_file ("dump-xml-file,d")
          , _Ono_inline ("no-inline")
          , _Osynthesize_virtual_places ("synthesize-virtual-places")
          , _Oforce_overwrite_file ("force-overwrite-file")
          , _Obackup_extension ("backup-extension")
          , _Odo_file_backup ("do-backup")

          , _Overbose_file ("verbose-file,v")
          , _Opath_to_cpp ("path-to-cpp,g")
        {}

        const search_path_type & search_path (void) const
        {
          return _search_path;
        }

        const gen_param_type& gen_ldflags (void) const
        {
          return _gen_ldflags;
        }

        const gen_param_type& gen_cxxflags (void) const
        {
          return _gen_cxxflags;
        }

        // ***************************************************************** //

        const ::xml::parse::type::requirements_type & requirements () const
        {
          return _requirements;
        }

        void set_requirement
        ( const ::xml::parse::type::require_key_type & key
        , const bool & mandatory
        )
        {
          _requirements.set (key, mandatory);
        }

        // ***************************************************************** //

        property::path_type & prop_path (void)
        {
          return _prop_path;
        }

        // ***************************************************************** //

        const context_t & context (void) const
        {
          return _context;
        }

        const optimize::options::type & options_optimize (void) const
        {
          return _options_optimize;
        }

        // ***************************************************************** //

        const key_values_t & key_values (void) const
        {
          return _key_values;
        }

        // ***************************************************************** //

        bool interpret_context_property ( const property::path_type & path
                                        , const property::value_type & value
                                        )
        {
          if (path.size() > 0 && path[0] == "pnetc")
            {
              if (path.size() > 2 && path[1] == "context")
                {
                  try
                    {
                      const value::type & old_val (_context.value (path[2]));

                      warn ( overwrite_context ( path[2]
                                               , value
                                               , old_val
                                               , file_in_progress()
                                               )
                           );
                    }
                  catch (const value::container::exception::missing_binding &)
                    {
                      /* do nothing, that's what we want */
                    }

                  std::ostringstream s;

                  s << "when try to bind context key "
                    << path[2] << " with " << value
                    << " in " << file_in_progress()
                    ;

                  const util::we_parser_t parser
                    ( util::generic_we_parse ( "${" + path[2] + "}:=" + value
                                             , s.str()
                                             )
                    );

                  try
                    {
                      parser.eval_all (_context);

                      _key_values.push_back (key_value_t (path[2], value));
                    }
                  catch (const expr::exception::eval::divide_by_zero & e)
                    {
                      throw error::eval_context_bind (s.str(), e.what());
                    }
                  catch (const expr::exception::eval::type_error & e)
                    {
                      throw error::eval_context_bind (s.str(), e.what());
                    }

                  return true;
                }
              else if (  path.size() > 2
                      && path[1] == "warning"
                      && path[2] == "inline-many-output-ports"
                      )
                {
                  /* do nothing, it's known */
                }
              else
                {
                  warn ( property_unknown ( path
                                          , value
                                          , file_in_progress()
                                          )
                       );
                }
            }

          return false;
        }

        // ***************************************************************** //

        void set_input (const fs::path & path)
        {
          _in_progress.push_back (path);
        }

        void set_input (const std::string & file)
        {
          set_input (fs::path (file));
        }

        fs::path file_in_progress (void) const
        {
          return (_in_progress.empty())
            ? fs::path("<stdin>")
            : _in_progress.back()
            ;
        }

        const std::string & path_to_cpp (void) const
        {
          return _path_to_cpp;
        }

        const std::string & dump_xml_file (void) const
        {
          return _dump_xml_file;
        }

        const std::string& backup_extension (void) const
        {
          return _backup_extension;
        }

        // ***************************************************************** //

        void dump_context (xml_util::xmlstream & s) const
        {
          typedef key_values_t::const_iterator it_t;

          it_t kv (_key_values.begin());
          const it_t end (_key_values.end());

          if (kv != end)
            {
              s.open ("properties"); s.attr ("name", "pnetc");
              s.open ("properties"); s.attr ("name", "context");

              while (kv != end)
                {
                  s.open ("property"); s.attr ("key", kv->key());

                  s.content (kv->value());

                  s.close();

                  ++kv;
                }

              s.close ();
              s.close ();
            }
        }

        // ***************************************************************** //

        void verbose (const std::string & msg) const
        {
          if (_verbose_file.size() > 0)
            {
              if (!_verbose_file_opened)
                {
                  _verbose_stream.open (_verbose_file.c_str());

                  if (!_verbose_stream.good())
                    {
                      throw error::could_not_open_file (_verbose_file);
                    }

                  _verbose_file_opened = true;
                }

              _verbose_stream << msg << std::endl;
            }
        }

        // ***************************************************************** //

#define ACCESST(_t,_x)                                  \
        const _t & _x (void) const { return _ ## _x; }  \
        _t & _x (void){ return _ ## _x; }
#define ACCESS(x) ACCESST(bool,x)

        ACCESS(ignore_properties)

        ACCESS(Werror)
        ACCESS(Wall)
        ACCESS(Woverwrite_function_name_as)
        ACCESS(Woverwrite_template_name_as)
        ACCESS(Wshadow_struct)
        ACCESS(Wshadow_function)
        ACCESS(Wdefault_construction)
        ACCESS(Wunused_field)
        ACCESS(Wport_not_connected)
        ACCESS(Wunexpected_element)
        ACCESS(Woverwrite_function_name_trans)
        ACCESS(Woverwrite_function_internal_trans)
        ACCESS(Wproperty_overwritten)
        ACCESS(Wtype_map_duplicate)
        ACCESS(Wtype_get_duplicate)
        ACCESS(Woverwrite_context)
        ACCESS(Windependent_place)
        ACCESS(Windependent_transition)
        ACCESS(Wconflicting_port_types)
        ACCESS(Woverwrite_file)
        ACCESS(Wbackup_file)
        ACCESS(Wduplicate_external_function)
        ACCESS(Wproperty_unknown)
        ACCESS(Winline_many_output_ports)

        ACCESS(no_inline)
        ACCESS(synthesize_virtual_places)
        ACCESS(force_overwrite_file)
        ACCESS(do_file_backup)

#undef ACCESS
#undef ACCESST

        // ***************************************************************** //

        template<typename T>
        void warn (const struct_shadowed<T> & w) const
        {
          generic_warn (w, _Wshadow_struct);
        }

#define WARN(x) void warn (const x & w) const { generic_warn (w, _W ## x); }

        WARN(overwrite_function_name_as)
        WARN(overwrite_template_name_as)
        WARN(default_construction)
        WARN(unused_field)
        WARN(port_not_connected)
        WARN(unexpected_element)
        WARN(overwrite_function_name_trans)
        WARN(overwrite_function_internal_trans)
        WARN(property_overwritten)
        WARN(type_map_duplicate)
        WARN(type_get_duplicate)
        WARN(overwrite_context)
        WARN(independent_place)
        WARN(independent_transition)
        WARN(conflicting_port_types)
        WARN(overwrite_file)
        WARN(backup_file)
        WARN(duplicate_external_function)
        WARN(property_unknown)
        WARN(inline_many_output_ports)
        WARN(shadow_function)

#undef WARN

        // ***************************************************************** //

        template<typename T>
        T generic_parse ( T (*parse)(std::istream &, type &)
                        , const boost::filesystem::path & path
                        )
        {
          _in_progress.push_back (path);

          std::ifstream stream (path.string().c_str());

          const T x (parse (stream, *this));

          _in_progress.pop_back ();

          return x;
        }

        template<typename T>
        T generic_parse ( T (*parse)(std::istream &, type &)
                        , const std::string & file
                        )
        {
          return generic_parse<T> (parse, boost::filesystem::path (file));
        }

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

          return generic_parse<T> (parse, path);
        };

        // ***************************************************************** //

        void add_options (po::options_description & desc)
        {
#define TYPEDVAL(t,x) po::value<t>(&_ ## x)->default_value (_ ## x)
#define BOOLVAL(x) TYPEDVAL(bool,x)
#define STRINGVAL(x) TYPEDVAL(std::string,x)

          desc.add_options ()
            ( _Osearch_path.c_str()
            , po::value<search_path_type>(&_search_path)
            , "search path"
            )
            ( _Ogen_ldflags.c_str()
            , po::value<gen_param_type>(&_gen_ldflags)
            , "ldflags for generated makefile"
            )
            ( _Ogen_cxxflags.c_str()
            , po::value<gen_param_type>(&_gen_cxxflags)
            , "cxxflags for generated makefile"
            )
            ( _Overbose_file.c_str()
            , STRINGVAL(verbose_file)
            , "verbose output goes there, empty for no verbose output"
            )
            ( _Opath_to_cpp.c_str()
            , STRINGVAL(path_to_cpp)
            , "path for cpp output, empty for no cpp output"
            )
            ( _Odump_xml_file.c_str()
            , STRINGVAL(dump_xml_file)
            , "file to dump the folded and pretty xml, empty for no dump"
            )
            ( _Oignore_properties.c_str()
            , BOOLVAL(ignore_properties)
            , "when set to true, no properties are parsed"
            )
            ( _OWerror.c_str()
            , BOOLVAL(Werror)
            , "cast warnings to errors"
            )
            ( _OWall.c_str()
            , BOOLVAL(Wall)
            , "turn on all warnings"
            )
            ( _OWoverwrite_function_name_as.c_str()
            , BOOLVAL(Woverwrite_function_name_as)
            , "warn when overwriting a function name by 'as'"
            )
            ( _OWoverwrite_template_name_as.c_str()
            , BOOLVAL(Woverwrite_template_name_as)
            , "warn when overwriting a template name by 'as'"
            )
            ( _OWshadow_struct.c_str()
            , BOOLVAL(Wshadow_struct)
            , "warn when shadowing a struct definition"
            )
            ( _OWshadow_function.c_str()
            , BOOLVAL(Wshadow_function)
            , "warn when shadowing a function definition"
            )
            ( _OWdefault_construction.c_str()
            , BOOLVAL(Wdefault_construction)
            , "warn when default construct (part of) tokens"
            )
            ( _OWunused_field.c_str()
            , BOOLVAL(Wunused_field)
            , "warn when given fields in tokens are unused"
            )
            ( _OWport_not_connected.c_str()
            , BOOLVAL(Wport_not_connected)
            , "warn when ports are not connected"
            )
            ( _OWunexpected_element.c_str()
            , BOOLVAL(Wunexpected_element)
            , "warn when unexpected elements occur"
            )
            ( _OWoverwrite_function_name_trans.c_str()
            , BOOLVAL(Woverwrite_function_name_trans)
            , "warn when overwriting a function name with a transition name"
            )
            ( _OWoverwrite_function_internal_trans.c_str()
            , BOOLVAL(Woverwrite_function_internal_trans)
            , "warn when overwriting a function internal flag by the transition internal flag"
            )
            ( _OWproperty_overwritten.c_str()
            , BOOLVAL(Wproperty_overwritten)
            , "warn when overwriting a property"
            )
            ( _OWtype_map_duplicate.c_str()
            , BOOLVAL(Wtype_map_duplicate)
            , "warn about duplicate type maps"
            )
            ( _OWtype_get_duplicate.c_str()
            , BOOLVAL(Wtype_get_duplicate)
            , "warn about duplicate type gets"
            )
            ( _OWoverwrite_context.c_str()
            , BOOLVAL(Woverwrite_context)
            , "warn when overwriting values in global context"
            )
            ( _OWindependent_place.c_str()
            , BOOLVAL(Windependent_place)
            , "warn when a place has no connection at all"
            )
            ( _OWindependent_transition.c_str()
            , BOOLVAL(Windependent_transition)
            , "warn when a transition has no connection at all"
            )
            ( _OWconflicting_port_types.c_str()
            , BOOLVAL(Wconflicting_port_types)
            , "warn when in/out port has conflicting types"
            )
            ( _OWoverwrite_file.c_str()
            , BOOLVAL(Woverwrite_file)
            , "warn when a file is overwritten"
            )
            ( _OWbackup_file.c_str()
            , BOOLVAL(Wbackup_file)
            , "warn when make a backup"
            )
            ( _OWduplicate_external_function.c_str()
            , BOOLVAL(Wduplicate_external_function)
            , "warn when an external function has multiple occurrences"
            )
            ( _OWproperty_unknown.c_str()
            , BOOLVAL(Wproperty_unknown)
            , "warn when a property is unknown"
            )
            ( _OWinline_many_output_ports.c_str()
            , BOOLVAL(Winline_many_output_ports)
            , "warn when a transition with more than one connected output"
              " port is inlined. This could lead to problems when the number"
              " of tokens is not the same on all output ports."
            )
            ( _Ono_inline.c_str()
            , BOOLVAL(no_inline)
            , "if set, ignore the keyword inline"
            )
            ( _Osynthesize_virtual_places.c_str()
            , BOOLVAL(synthesize_virtual_places)
            , "if set, synthesize virtual places"
            )
            ( _Oforce_overwrite_file.c_str()
            , BOOLVAL(force_overwrite_file)
            , "force overwrite files"
            )
            ( _Obackup_extension.c_str()
            , STRINGVAL(backup_extension)
            , "backup extension"
            )
            ( _Odo_file_backup.c_str()
            , BOOLVAL(do_file_backup)
            , "make backup copies of files before overwriting"
            )
            ;
#undef TYPEDVAL
#undef BOOLVAL
#undef STRINGVAL

          _options_optimize.add_options (desc);
        }
      };
    }
  }
}

#endif
