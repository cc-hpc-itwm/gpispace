// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/state.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/util/weparse.hpp>

namespace xml
{
  namespace parse
  {
    namespace state
    {
      namespace fs = boost::filesystem;
      namespace property = we::type::property;
      namespace optimize = we::type::optimize;

      using namespace warning;

      typedef std::vector<std::string> search_path_type;
      typedef std::vector<fs::path> in_progress_type;
      typedef std::set<fs::path> dependencies_type;

      typedef std::vector<std::string> gen_param_type;

      // ******************************************************************* //

      key_value_t::key_value_t ( const std::string & key
                               , const std::string & value
                               )
        : _key (key)
        , _value (value)
      {}

      const std::string & key_value_t::key () const { return _key; }
      const std::string & key_value_t::value () const { return _value; }

      // ******************************************************************* //

      template<typename W>
      void type::generic_warn ( const W & w
                              , const bool & active
                              , const std::string & flag
                              ) const
      {
        if (_Wall || active)
        {
          if (_Werror)
          {
            throw w;
          }
          else
          {
            std::cerr << w.what() << " [--" << flag << "]" << std::endl;
          }
        }
      }

      fs::path type::expand (const std::string & file) const
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

      type::type (void)
        : _search_path ()
        , _gen_ldflags ()
        , _gen_cxxflags ()
        , _in_progress ()
        , _dependencies ()
        , _ignore_properties (false)
        , _Werror (false)
        , _Wall (false)
        , _Woverwrite_function_name_as (false)
        , _Woverwrite_template_name_as (false)
        , _Wshadow_struct (true)
        , _Wshadow_function (true)
        , _Wshadow_template (true)
        , _Wshadow_specialize (true)
        , _Wdefault_construction (true)
        , _Wunused_field (true)
        , _Wport_not_connected (true)
        , _Wunexpected_element (true)
        , _Woverwrite_function_name_trans (false)
        , _Woverwrite_function_internal_trans (true)
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
        , _Wvirtual_place_not_tunneled (true)
        , _Wduplicate_template_parameter (true)

        , _dump_xml_file ("")
        , _dump_dependencies ("")
        , _list_dependencies ("")
        , _dump_dependenciesD ()
        , _dependencies_target ()
        , _dependencies_target_quoted ()
        , _dependencies_add_phony_targets ()
        , _no_inline (false)
        , _synthesize_virtual_places (false)
        , _force_overwrite_file (false)
        , _backup_extension ("~")
        , _do_file_backup (true)

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
        , _OWshadow_template ("Wshadow-template")
        , _OWshadow_specialize ("Wshadow-specialize")
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
        , _OWvirtual_place_not_tunneled ("Wvirtual-place-not-tunneled")
        , _OWduplicate_template_parameter ("Wduplicate-template-parameter")

        , _Odump_xml_file ("dump-xml-file,d")
        , _Odump_dependencies ("dump-dependencies,M")
        , _Olist_dependencies ("list-dependencies")
        , _Odump_dependenciesD ("dump-dependenciesD")
        , _Odependencies_target ("dependencies-target")
        , _Odependencies_target_quoted ("dependencies-target-quoted")
        , _Odependencies_add_phony_targets ("dependencies-add-phony-targets")
        , _Ono_inline ("no-inline")
        , _Osynthesize_virtual_places ("synthesize-virtual-places")
        , _Oforce_overwrite_file ("force-overwrite-file")
        , _Obackup_extension ("backup-extension")
        , _Odo_file_backup ("do-backup")

        , _Opath_to_cpp ("path-to-cpp,g")

        , _id_mapper()
      {}

      const search_path_type & type::search_path (void) const
      {
        return _search_path;
      }

      const gen_param_type& type::gen_ldflags (void) const
      {
        return _gen_ldflags;
      }

      const gen_param_type& type::gen_cxxflags (void) const
      {
        return _gen_cxxflags;
      }

      gen_param_type& type::gen_ldflags (void)
      {
        return _gen_ldflags;
      }

      gen_param_type& type::gen_cxxflags (void)
      {
        return _gen_cxxflags;
      }

      // ***************************************************************** //

      const ::xml::parse::type::requirements_type & type::requirements () const
      {
        return _requirements;
      }

      void type::set_requirement
        ( const ::xml::parse::type::require_key_type & key
        , const bool & mandatory
        )
      {
        _requirements.set (key, mandatory);
      }

      // ***************************************************************** //

      property::path_type & type::prop_path (void)
      {
        return _prop_path;
      }

      // ***************************************************************** //

      const type::context_t & type::context (void) const
      {
        return _context;
      }

      const optimize::options::type & type::options_optimize (void) const
      {
        return _options_optimize;
      }

      // ***************************************************************** //

      const key_values_t & type::key_values (void) const
      {
        return _key_values;
      }

      // ***************************************************************** //

      bool type::interpret_context_property ( const property::path_type & path
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

      void type::set_input (const fs::path & path)
      {
        _in_progress.push_back (path);
      }

      void type::set_input (const std::string & file)
      {
        set_input (fs::path (file));
      }

      fs::path type::file_in_progress (void) const
      {
        return (_in_progress.empty())
          ? fs::path("<stdin>")
          : _in_progress.back()
          ;
      }

      const dependencies_type& type::dependencies (void) const
      {
        return _dependencies;
      }

#define ACCESS(name)                                                    \
      const std::string & type::name (void) const { return _ ## name; } \
      std::string & type::name (void) { return _ ## name; }

      ACCESS(path_to_cpp)
      ACCESS(dump_xml_file)
      ACCESS(dump_dependencies)
      ACCESS(list_dependencies)
      ACCESS(backup_extension)

#undef ACCESS

#define ACCESS(name) const std::vector<std::string> & type::name (void) const { return _ ## name; }

      ACCESS(dependencies_target)
      ACCESS(dependencies_target_quoted)

#undef ACCESS

      // ***************************************************************** //

      void type::dump_context (::fhg::util::xml::xmlstream & s) const
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

#define ACCESST(_t,_x)                                     \
      const _t & type::_x (void) const { return _ ## _x; } \
      _t & type::_x (void){ return _ ## _x; }
#define ACCESS(x) ACCESST(bool,x)

      ACCESS(ignore_properties)

      ACCESS(Werror)
      ACCESS(Wall)
      ACCESS(Woverwrite_function_name_as)
      ACCESS(Woverwrite_template_name_as)
      ACCESS(Wshadow_struct)
      ACCESS(Wshadow_function)
      ACCESS(Wshadow_template)
      ACCESS(Wshadow_specialize)
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
      ACCESS(Wvirtual_place_not_tunneled)
      ACCESS(Wduplicate_template_parameter)

      ACCESS(no_inline)
      ACCESS(synthesize_virtual_places)
      ACCESS(force_overwrite_file)
      ACCESS(do_file_backup)

      ACCESS(dependencies_add_phony_targets)
      ACCESS(dump_dependenciesD)

#undef ACCESS
#undef ACCESST

      // ***************************************************************** //

#define WARN(x)                                                         \
      void type::warn (const x & w) const                               \
      { generic_warn (w, _W ## x, _OW ## x ); }
#define WARN_(x,y)                                                      \
      void type::warn (const x & w) const                               \
      { generic_warn (w, _W ## y, _OW ## y ); }

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
      WARN(virtual_place_not_tunneled)
      WARN(shadow_function)
      WARN(shadow_template)
      WARN(shadow_specialize)
      WARN(duplicate_template_parameter)
      WARN_(struct_shadowed,shadow_struct)

#undef WARN
#undef WARN_

      // ***************************************************************** //


      void type::check_for_include_loop(const fs::path& path) const
      {
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
      }

      namespace po = boost::program_options;

      void type::add_options (po::options_description & desc)
      {
#define TYPEDVAL(t,x) po::value<t>(&_ ## x)->default_value (_ ## x)
#define BOOLVAL(x) TYPEDVAL(bool,x)->implicit_value(true)
#define STRINGVAL(x) TYPEDVAL(std::string,x)
#define STRINGVECVAL(x) po::value<std::vector<std::string> >(&_ ## x)

        po::options_description warnings ("Warnings");

        warnings.add_options ()
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
          ( _OWshadow_template.c_str()
          , BOOLVAL(Wshadow_template)
          , "warn when shadowing a template definition"
          )
          ( _OWshadow_specialize.c_str()
          , BOOLVAL(Wshadow_specialize)
          , "warn when shadowing a specialization"
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
          ( _OWvirtual_place_not_tunneled.c_str()
          , BOOLVAL(Wvirtual_place_not_tunneled)
          , "warn when a virtual place is not tunneled"
          )
          ( _OWduplicate_template_parameter.c_str()
          , BOOLVAL(Wduplicate_template_parameter)
          , "warn when a template paramater is duplicated"
          )
          ;

        po::options_description generate ("Wrapper generation");

        generate.add_options ()
          ( _Ogen_ldflags.c_str()
          , po::value<gen_param_type>(&_gen_ldflags)
          , "ldflags for generated makefile"
          )
          ( _Ogen_cxxflags.c_str()
          , po::value<gen_param_type>(&_gen_cxxflags)
          , "cxxflags for generated makefile"
          )
          ( _Opath_to_cpp.c_str()
          , STRINGVAL(path_to_cpp)->implicit_value ("gen")
          , "path for cpp output, empty for no cpp output"
          )
          ;

        po::options_description xml ("XML mirroring");

        xml.add_options ()
          ( _Odump_xml_file.c_str()
          , STRINGVAL(dump_xml_file)->implicit_value("/dev/stdout")
          , "file to dump the folded and pretty xml, empty for no dump"
          )
          ;

        po::options_description depend ("Dependency generation");

        depend.add_options ()
          ( _Odump_dependencies.c_str()
          , STRINGVAL(dump_dependencies)->implicit_value("/dev/stdout")
          , "file to dump the dependencies as Make target, empty for no dump"
          " (also as -MM -MF -MG)"
          )
          ( _Odump_dependenciesD.c_str()
          , BOOLVAL(dump_dependenciesD)
          , "dump the dependencies as Make target to <input>.d"
          " (also as -MD -MMD)"
          )
          ( _Odependencies_target.c_str()
          , STRINGVECVAL(dependencies_target)
          , "set the target in the dependency file (also as -MT)"
          )
          ( _Odependencies_target_quoted.c_str()
          , STRINGVECVAL(dependencies_target_quoted)
          , "like -MT but quote special characters (also as -MQ)"
          )
          ( _Odependencies_add_phony_targets.c_str()
          , BOOLVAL(dependencies_add_phony_targets)
          , "add phony targets for all dependencies (also as -MP)"
          )
          ( _Olist_dependencies.c_str()
          , STRINGVAL(list_dependencies)->implicit_value("/dev/stdout")
          , "file to list the dependencies, empty for no list"
          )
          ;

        po::options_description net ("Network handling");

        net.add_options ()
          ( _Oignore_properties.c_str()
          , BOOLVAL(ignore_properties)
          , "when set to true, no properties are parsed"
          )
          ( _Ono_inline.c_str()
          , BOOLVAL(no_inline)
          , "if set, ignore the keyword inline"
          )
          ( _Osynthesize_virtual_places.c_str()
          , BOOLVAL(synthesize_virtual_places)
          , "if set, synthesize virtual places"
          )
          ;

        po::options_description file ("File handling");

        file.add_options ()
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

        desc.add_options ()
          ( _Osearch_path.c_str()
          , po::value<search_path_type>(&_search_path)
          , "search path"
          )
          ;
#undef TYPEDVAL
#undef BOOLVAL
#undef STRINGVAL

        desc.add (net);
        _options_optimize.add_options (desc);

        desc.add (xml);
        desc.add (depend);
        desc.add (generate);
        desc.add (file);
        desc.add (warnings);
      }

      const id::mapper* type::id_mapper() const
      {
        return &_id_mapper;
      }
      id::mapper* type::id_mapper()
      {
        return &_id_mapper;
      }

      namespace
      {
        typedef std::pair<std::string, std::string> pair_type;

        pair_type mk (const std::string & param)
        {
          return std::make_pair (param, std::string());
        }
      }

      pair_type reg_M (const std::string& s)
      {
        std::string::const_iterator pos (s.begin());
        const std::string::const_iterator end (s.end());

        if (pos != end)
        {
          switch (*pos)
          {
          case '-':
            if (++pos != end)
            {
              switch (*pos)
              {
              case 'M':
                if (++pos == end)
                {
                  return mk ("dump-dependencies");
                }
                else
                {
                  switch (*pos)
                  {
                  case 'M':
                    if (++pos == end)
                    {
                      return mk ("dump-dependencies");
                    }
                    else
                    {
                      switch (*pos)
                      {
                      case 'D':
                        return mk ("dump-dependenciesD");
                      default: break;
                      }
                    }
                    break;
                  case 'D':
                    return mk ("dump-dependenciesD");
                  case 'F':
                  case 'G':
                    return mk ("dump-dependencies");
                  case 'T':
                    return mk ("dependencies-target");
                  case 'Q':
                    return mk ("dependencies-target-quoted");
                  case 'P':
                    return mk ("dependencies-add-phony-targets");
                  default: break;
                  }
                }
                break;
              default: break;
              }
            }
            break;
          default: break;
          }
        }

        return mk ("");
      }
    }
  }
}
