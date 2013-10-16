// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/state.hpp>
#include <xml/parse/error.hpp>

#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

namespace xml
{
  namespace parse
  {
    namespace state
    {
      template<typename W>
      void type::generic_warn ( const W& w
                              , const bool& active
                              , const std::string& flag
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

      boost::filesystem::path type::expand (const std::string& file) const
      {
        const boost::filesystem::path absolute (file);

        if (absolute.is_absolute())
        {
          if (boost::filesystem::exists (absolute))
          {
            return absolute;
          }
        }
        else
        {
          const boost::filesystem::path from_actual_file
            (file_in_progress().parent_path() / file);

          if (boost::filesystem::exists (from_actual_file))
          {
            return from_actual_file;
          }
          else
          {
            BOOST_FOREACH (const std::string& search_path, _search_path)
            {
              if (!boost::filesystem::exists (search_path))
              {
                continue;
              }

              const boost::filesystem::path pre (search_path);
              const boost::filesystem::path path (pre / file);

              if (boost::filesystem::exists (path))
              {
                return path;
              }
            }
          }
        }

        throw error::file_not_found ("expand", file);
      }

      type::type()
        : _search_path()
        , _gen_ldflags()
        , _gen_cxxflags()
        , _in_progress()
        , _in_progress_position()
        , _dependencies()
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
        , _Wsynthesize_anonymous_function (true)

        , _dump_xml_file ("")
        , _dump_dependencies ("")
        , _list_dependencies ("")
        , _dump_dependenciesD()
        , _dependencies_target()
        , _dependencies_target_quoted()
        , _dependencies_add_phony_targets()
        , _no_inline (false)
        , _synthesize_virtual_places (false)
        , _force_overwrite_file (false)
        , _backup_extension ("~")
        , _do_file_backup (true)

        , _path_to_cpp ("")
        , _link_prefix()
        , _link_prefix_by_key()
        , _link_prefix_parsed (false)

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
        , _OWsynthesize_anonymous_function ("Wsynthesize-anonymous-function")

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
        , _Olink_prefix("link-prefix")

        , _id_mapper()
      {}

      const gen_param_type& type::gen_ldflags() const
      {
        return _gen_ldflags;
      }

      const gen_param_type& type::gen_cxxflags() const
      {
        return _gen_cxxflags;
      }

      gen_param_type& type::gen_ldflags()
      {
        return _gen_ldflags;
      }

      gen_param_type& type::gen_cxxflags()
      {
        return _gen_cxxflags;
      }

      const link_prefix_type& type::link_prefix() const
      {
        return _link_prefix;
      }

      const std::string&
      type::link_prefix_by_key (const std::string& key) const
      {
        if (!_link_prefix_parsed)
        {
          BOOST_FOREACH (const std::string& kv, _link_prefix)
          {
            std::string parsed_key;
            fhg::util::parse::position inp (kv.begin(), kv.end());
            bool found_eq (false);

            while (!found_eq && !inp.end())
            {
              if (*inp == '=')
              {
                found_eq = true;
              }
              else
              {
                parsed_key += *inp;
              }

              ++inp;
            }

            if (!parsed_key.size())
            {
              throw error::parse_link_prefix ("Missing key", kv, inp.eaten());
            }

            if (!found_eq)
            {
              throw error::parse_link_prefix ("Missing =", kv, inp.eaten());
            }

            if (inp.end())
            {
              throw error::parse_link_prefix ("Missing value", kv, inp.eaten());
            }

            _link_prefix_by_key[parsed_key] =
              fhg::util::parse::require::rest (inp);
          }

          _link_prefix_parsed = true;
        }

        const boost::unordered_map<std::string, std::string>::const_iterator
          pos (_link_prefix_by_key.find (key));

        if (pos != _link_prefix_by_key.end())
        {
          return pos->second;
        }

        throw error::link_prefix_missing (key);
      }

      const ::xml::parse::type::requirements_type& type::requirements() const
      {
        return _requirements;
      }

      void type::set_requirement
        ( const ::xml::parse::type::require_key_type& key
        , const bool& mandatory
        )
      {
        _requirements.set (key, mandatory);
      }

      we::type::property::path_type& type::prop_path()
      {
        return _prop_path;
      }

      const we::type::optimize::options::type& type::options_optimize() const
      {
        return _options_optimize;
      }

      void
      type::interpret_property ( const we::type::property::path_type& path
                               , const we::type::property::value_type& value
                               )
      {
        if (path.size() > 0 && path[0] == "pnetc")
        {
          if (path.size() > 1 && path[1] == "search_path")
          {
            const boost::filesystem::path absolute (value);

            if (absolute.is_absolute())
            {
              if (boost::filesystem::exists (absolute))
              {
                _search_path.push_back (absolute.string());
              }
              else
              {
                throw error::file_not_found
                  ("interpret_property (pnetc.search_path)", value);
              }
            }
            else
            {
              const boost::filesystem::path from_actual_file
                (file_in_progress().parent_path() / value);

              if (boost::filesystem::exists (from_actual_file))
              {
                _search_path.push_back (from_actual_file.string());
              }
              else
              {
                throw error::file_not_found
                  ("interpret_property (pnetc.search_path)", value);
              }
            }
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
            warn ( warning::property_unknown ( path
                                             , value
                                             , file_in_progress()
                                             )
                 );
          }
        }
      }

      void type::set_input (const boost::filesystem::path& path)
      {
        _in_progress.push_back (path);
      }

      void type::set_input (const std::string& file)
      {
        set_input (boost::filesystem::path (file));
      }

      boost::filesystem::path type::file_in_progress() const
      {
        return (_in_progress.empty())
          ? boost::filesystem::path("<stdin>")
          : _in_progress.back()
          ;
      }

      void type::set_in_progress_position (const char* p)
      {
        in_progress_position_type::iterator m
          (_in_progress_position.insert ( _in_progress_position.end()
                                        , position_by_pointer_type()
                                        )
          );

        m->insert
          (std::make_pair ( p
                          , util::position_type (p, p, file_in_progress())
                          )
          );
      }
      util::position_type type::position (const xml_node_type* node) const
      {
        if (_in_progress_position.empty())
        {
          throw std::runtime_error ("start position requested but unknown");
        }

        position_by_pointer_type& m (_in_progress_position.back());

        const position_by_pointer_type::const_iterator before
          (m.lower_bound (node->name()));

        if (before == m.end())
        {
          throw std::runtime_error ("no prior position found");
        }

        const util::position_type p ( before->first
                                    , node->name()
                                    , file_in_progress()
                                    , before->second.line()
                                    , before->second.column()
                                    );

        m.insert (std::make_pair (node->name(), p));

        return p;
      }

      const std::set<boost::filesystem::path>& type::dependencies() const
      {
        return _dependencies;
      }

#define ACCESS(name)                                                    \
      const std::string& type::name() const                             \
      {                                                                 \
        return _ ## name;                                               \
      }                                                                 \
      std::string& type::name()                                         \
      {                                                                 \
        return _ ## name;                                               \
      }

      ACCESS (path_to_cpp)
      ACCESS (dump_xml_file)
      ACCESS (dump_dependencies)
      ACCESS (list_dependencies)
      ACCESS (backup_extension)

#undef ACCESS

#define ACCESS(name)                                                    \
        const std::vector<std::string>& type::name() const              \
        {                                                               \
          return _ ## name;                                             \
        }

      ACCESS (dependencies_target)
      ACCESS (dependencies_target_quoted)

#undef ACCESS

#define ACCESST(_t,_x)                          \
        const _t& type::_x() const              \
        {                                       \
          return _ ## _x;                       \
        }                                       \
        _t& type::_x()                          \
        {                                       \
          return _ ## _x;                       \
        }
#define ACCESS(x) ACCESST(bool,x)

      ACCESS (ignore_properties)

      ACCESS (Werror)
      ACCESS (Wall)
      ACCESS (Woverwrite_function_name_as)
      ACCESS (Woverwrite_template_name_as)
      ACCESS (Wshadow_struct)
      ACCESS (Wshadow_function)
      ACCESS (Wshadow_template)
      ACCESS (Wshadow_specialize)
      ACCESS (Wdefault_construction)
      ACCESS (Wunused_field)
      ACCESS (Wport_not_connected)
      ACCESS (Wunexpected_element)
      ACCESS (Woverwrite_function_name_trans)
      ACCESS (Woverwrite_function_internal_trans)
      ACCESS (Wproperty_overwritten)
      ACCESS (Wtype_map_duplicate)
      ACCESS (Wtype_get_duplicate)
      ACCESS (Windependent_place)
      ACCESS (Windependent_transition)
      ACCESS (Wconflicting_port_types)
      ACCESS (Woverwrite_file)
      ACCESS (Wbackup_file)
      ACCESS (Wduplicate_external_function)
      ACCESS (Wproperty_unknown)
      ACCESS (Winline_many_output_ports)
      ACCESS (Wvirtual_place_not_tunneled)
      ACCESS (Wduplicate_template_parameter)
      ACCESS (Wsynthesize_anonymous_function)

      ACCESS (no_inline)
      ACCESS (synthesize_virtual_places)
      ACCESS (force_overwrite_file)
      ACCESS (do_file_backup)

      ACCESS (dependencies_add_phony_targets)
      ACCESS (dump_dependenciesD)

#undef ACCESS
#undef ACCESST

#define WARN(x)                                                         \
        void type::warn (const warning::x& w) const                     \
        {                                                               \
          generic_warn (w, _W ## x, _OW ## x );                         \
        }
#define WARN_(x,y)                                                      \
      void type::warn (const warning::x& w) const                       \
      {                                                                 \
        generic_warn (w, _W ## y, _OW ## y );                           \
      }

      WARN (overwrite_function_name_as)
      WARN (overwrite_template_name_as)
      WARN (default_construction)
      WARN (unused_field)
      WARN (port_not_connected)
      WARN (unexpected_element)
      WARN (overwrite_function_name_trans)
      WARN (overwrite_function_internal_trans)
      WARN (property_overwritten)
      WARN (type_map_duplicate)
      WARN (type_get_duplicate)
      WARN (independent_place)
      WARN (independent_transition)
      WARN (conflicting_port_types)
      WARN (overwrite_file)
      WARN (backup_file)
      WARN (duplicate_external_function)
      WARN (property_unknown)
      WARN (inline_many_output_ports)
      WARN (virtual_place_not_tunneled)
      WARN (shadow_function)
      WARN (shadow_template)
      WARN (shadow_specialize)
      WARN (duplicate_template_parameter)
      WARN_(struct_shadowed,shadow_struct)
      WARN (synthesize_anonymous_function)

#undef WARN
#undef WARN_

      void
      type::check_for_include_loop (const boost::filesystem::path& path) const
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

      void type::add_options (boost::program_options::options_description& desc)
      {
#define TYPEDVAL(t,x)                                                   \
        boost::program_options::value<t>(&_ ## x)->default_value (_ ## x)
#define BOOLVAL(x)                              \
        TYPEDVAL (bool,x)->implicit_value(true)
#define STRINGVAL(x)                            \
        TYPEDVAL (std::string,x)
#define STRINGVECVAL(x)                                                 \
        boost::program_options::value<std::vector<std::string> > (&_ ## x)

        boost::program_options::options_description warnings ("Warnings");

        warnings.add_options()
          ( _OWerror.c_str()
          , BOOLVAL (Werror)
          , "cast warnings to errors"
          )
          ( _OWall.c_str()
          , BOOLVAL (Wall)
          , "turn on all warnings"
          )
          ( _OWoverwrite_function_name_as.c_str()
          , BOOLVAL (Woverwrite_function_name_as)
          , "warn when overwriting a function name by 'as'"
          )
          ( _OWoverwrite_template_name_as.c_str()
          , BOOLVAL (Woverwrite_template_name_as)
          , "warn when overwriting a template name by 'as'"
          )
          ( _OWshadow_struct.c_str()
          , BOOLVAL (Wshadow_struct)
          , "warn when shadowing a struct definition"
          )
          ( _OWshadow_function.c_str()
          , BOOLVAL (Wshadow_function)
          , "warn when shadowing a function definition"
          )
          ( _OWshadow_template.c_str()
          , BOOLVAL (Wshadow_template)
          , "warn when shadowing a template definition"
          )
          ( _OWshadow_specialize.c_str()
          , BOOLVAL (Wshadow_specialize)
          , "warn when shadowing a specialization"
          )
          ( _OWdefault_construction.c_str()
          , BOOLVAL (Wdefault_construction)
          , "warn when default construct (part of) tokens"
          )
          ( _OWunused_field.c_str()
          , BOOLVAL (Wunused_field)
          , "warn when given fields in tokens are unused"
          )
          ( _OWport_not_connected.c_str()
          , BOOLVAL (Wport_not_connected)
          , "warn when ports are not connected"
          )
          ( _OWunexpected_element.c_str()
          , BOOLVAL (Wunexpected_element)
          , "warn when unexpected elements occur"
          )
          ( _OWoverwrite_function_name_trans.c_str()
          , BOOLVAL (Woverwrite_function_name_trans)
          , "warn when overwriting a function name with a transition name"
          )
          ( _OWoverwrite_function_internal_trans.c_str()
          , BOOLVAL (Woverwrite_function_internal_trans)
          , "warn when overwriting a function internal flag by the transition internal flag"
          )
          ( _OWproperty_overwritten.c_str()
          , BOOLVAL (Wproperty_overwritten)
          , "warn when overwriting a property"
          )
          ( _OWtype_map_duplicate.c_str()
          , BOOLVAL (Wtype_map_duplicate)
          , "warn about duplicate type maps"
          )
          ( _OWtype_get_duplicate.c_str()
          , BOOLVAL (Wtype_get_duplicate)
          , "warn about duplicate type gets"
          )
          ( _OWindependent_place.c_str()
          , BOOLVAL (Windependent_place)
          , "warn when a place has no connection at all"
          )
          ( _OWindependent_transition.c_str()
          , BOOLVAL (Windependent_transition)
          , "warn when a transition has no connection at all"
          )
          ( _OWconflicting_port_types.c_str()
          , BOOLVAL (Wconflicting_port_types)
          , "warn when in/out port has conflicting types"
          )
          ( _OWoverwrite_file.c_str()
          , BOOLVAL (Woverwrite_file)
          , "warn when a file is overwritten"
          )
          ( _OWbackup_file.c_str()
          , BOOLVAL (Wbackup_file)
          , "warn when make a backup"
          )
          ( _OWduplicate_external_function.c_str()
          , BOOLVAL (Wduplicate_external_function)
          , "warn when an external function has multiple occurrences"
          )
          ( _OWproperty_unknown.c_str()
          , BOOLVAL (Wproperty_unknown)
          , "warn when a property is unknown"
          )
          ( _OWinline_many_output_ports.c_str()
          , BOOLVAL (Winline_many_output_ports)
          , "warn when a transition with more than one connected output"
            " port is inlined. This could lead to problems when the number"
            " of tokens is not the same on all output ports."
          )
          ( _OWvirtual_place_not_tunneled.c_str()
          , BOOLVAL (Wvirtual_place_not_tunneled)
          , "warn when a virtual place is not tunneled"
          )
          ( _OWduplicate_template_parameter.c_str()
          , BOOLVAL (Wduplicate_template_parameter)
          , "warn when a template paramater is duplicated"
          )
          ( _OWsynthesize_anonymous_function.c_str()
          , BOOLVAL (Wsynthesize_anonymous_function)
          , "warn when a anonymous top level function is synthesized"
          )
          ;

        boost::program_options::options_description
          generate ("Wrapper generation");

        generate.add_options()
          ( _Ogen_ldflags.c_str()
          , boost::program_options::value<gen_param_type>(&_gen_ldflags)
          , "ldflags for generated makefile"
          )
          ( _Ogen_cxxflags.c_str()
          , boost::program_options::value<gen_param_type>(&_gen_cxxflags)
          , "cxxflags for generated makefile"
          )
          ( _Opath_to_cpp.c_str()
          , STRINGVAL (path_to_cpp)->implicit_value ("gen")
          , "path for cpp output, empty for no cpp output"
          )
          ( _Olink_prefix.c_str()
          , boost::program_options::value<link_prefix_type>(&_link_prefix)
          , "prefix for linking, key=value"
          )
          ;

        boost::program_options::options_description xml ("XML mirroring");

        xml.add_options()
          ( _Odump_xml_file.c_str()
          , STRINGVAL (dump_xml_file)->implicit_value("/dev/stdout")
          , "file to dump the folded and pretty xml, empty for no dump"
          )
          ;

        boost::program_options::options_description
          depend ("Dependency generation");

        depend.add_options()
          ( _Odump_dependencies.c_str()
          , STRINGVAL (dump_dependencies)->implicit_value("/dev/stdout")
          , "file to dump the dependencies as Make target, empty for no dump"
          " (also as -MM -MF -MG)"
          )
          ( _Odump_dependenciesD.c_str()
          , BOOLVAL (dump_dependenciesD)
          , "dump the dependencies as Make target to <input>.d"
          " (also as -MD -MMD)"
          )
          ( _Odependencies_target.c_str()
          , STRINGVECVAL (dependencies_target)
          , "set the target in the dependency file (also as -MT)"
          )
          ( _Odependencies_target_quoted.c_str()
          , STRINGVECVAL (dependencies_target_quoted)
          , "like -MT but quote special characters (also as -MQ)"
          )
          ( _Odependencies_add_phony_targets.c_str()
          , BOOLVAL (dependencies_add_phony_targets)
          , "add phony targets for all dependencies (also as -MP)"
          )
          ( _Olist_dependencies.c_str()
          , STRINGVAL (list_dependencies)->implicit_value("/dev/stdout")
          , "file to list the dependencies, empty for no list"
          )
          ;

        boost::program_options::options_description net ("Network handling");

        net.add_options()
          ( _Oignore_properties.c_str()
          , BOOLVAL (ignore_properties)
          , "when set to true, no properties are parsed"
          )
          ( _Ono_inline.c_str()
          , BOOLVAL (no_inline)
          , "if set, ignore the keyword inline"
          )
          ( _Osynthesize_virtual_places.c_str()
          , BOOLVAL (synthesize_virtual_places)
          , "if set, synthesize virtual places"
          )
          ;

        boost::program_options::options_description file ("File handling");

        file.add_options()
          ( _Oforce_overwrite_file.c_str()
          , BOOLVAL (force_overwrite_file)
          , "force overwrite files"
          )
          ( _Obackup_extension.c_str()
          , STRINGVAL (backup_extension)
          , "backup extension"
          )
          ( _Odo_file_backup.c_str()
          , BOOLVAL (do_file_backup)
          , "make backup copies of files before overwriting"
          )
          ;

        desc.add_options()
          ( _Osearch_path.c_str()
          , boost::program_options::value<search_path_type>(&_search_path)
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

        pair_type mk (const std::string& param)
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
