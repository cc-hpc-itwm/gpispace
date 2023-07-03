// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>

#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <iterator>

namespace xml
{
  namespace parse
  {
    namespace state
    {
      template<typename W>
      void type::generic_warn ( W const& w
                              , bool const& active
                              , std::string const& flag
                              ) const
      {
        if (_warning_all || active)
        {
          if (_warning_error)
          {
            throw w;
          }
          else
          {
            std::cerr << w.what() << " [--" << flag << "]" << std::endl;
          }
        }
      }

      ::boost::filesystem::path type::expand (std::string const& file) const
      {
        const ::boost::filesystem::path absolute (file);

        if (absolute.is_absolute())
        {
          if (::boost::filesystem::exists (absolute))
          {
            return absolute;
          }
        }
        else
        {
          const ::boost::filesystem::path from_actual_file
            (file_in_progress().parent_path() / file);

          if (::boost::filesystem::exists (from_actual_file))
          {
            return from_actual_file;
          }
          else
          {
            for (std::string const& search_path : _search_path)
            {
              if (!::boost::filesystem::exists (search_path))
              {
                continue;
              }

              const ::boost::filesystem::path pre (search_path);
              const ::boost::filesystem::path path (pre / file);

              if (::boost::filesystem::exists (path))
              {
                return path;
              }
            }
          }
        }

        throw error::file_not_found ("expand", file);
      }

      gen_param_type const& type::gen_ldflags() const
      {
        return _gen_ldflags;
      }

      gen_param_type const& type::gen_cxxflags() const
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

      const ::xml::parse::type::requirements_type& type::requirements() const
      {
        return _requirements;
      }

      void type::set_requirement
        (const ::xml::parse::type::require_key_type& key)
      {
        _requirements.set (key);
      }

      we::type::property::path_type& type::prop_path()
      {
        return _prop_path;
      }

      void
      type::interpret_property ( we::type::property::path_type const& path
                               , we::type::property::value_type const& value
                               )
      {
        auto pos (path.begin());
        we::type::property::path_type::const_iterator const end (path.end());

        if (pos != end && *pos == "pnetc")
        {
          ++pos;

          if (pos != end && *pos == "search_path")
          {
            std::string const value_str (::boost::get<std::string> (value));
            const ::boost::filesystem::path absolute (value_str);

            if (absolute.is_absolute())
            {
              if (::boost::filesystem::exists (absolute))
              {
                _search_path.push_back (absolute.string());
              }
              else
              {
                throw error::file_not_found
                  ("interpret_property (pnetc.search_path)", value_str);
              }
            }
            else
            {
              const ::boost::filesystem::path from_actual_file
                (file_in_progress().parent_path() / value_str);

              if (::boost::filesystem::exists (from_actual_file))
              {
                _search_path.push_back (from_actual_file.string());
              }
              else
              {
                throw error::file_not_found
                  ("interpret_property (pnetc.search_path)", value_str);
              }
            }
          }
          else if (  pos != end
                  && *pos == "warning"
                  && std::next (pos) != end
                  && *std::next (pos) == "inline_many_output_ports"
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

      void type::set_input (::boost::filesystem::path const& path)
      {
        _in_progress.push_back (path);
      }

      void type::set_input (std::string const& file)
      {
        set_input (::boost::filesystem::path (file));
      }

      ::boost::filesystem::path type::file_in_progress() const
      {
        return (_in_progress.empty())
          ? ::boost::filesystem::path("<stdin>")
          : _in_progress.back()
          ;
      }

      void type::set_in_progress_position (const char* p)
      {
        auto m
          (_in_progress_position.insert ( _in_progress_position.end()
                                        , position_by_pointer_type()
                                        )
          );

        m->emplace (p, util::position_type (p, p, file_in_progress()));
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

        m.emplace (node->name(), p);

        return p;
      }

      std::set<::boost::filesystem::path> const& type::dependencies() const
      {
        return _dependencies;
      }

#define ACCESS(name)                                                    \
      std::string const& type::name() const                             \
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
        std::vector<std::string> const& type::name() const              \
        {                                                               \
          return _ ## name;                                             \
        }

      ACCESS (dependencies_target)
      ACCESS (dependencies_target_quoted)

#undef ACCESS

#define ACCESST(_t,_x)                          \
        _t const& type::_x() const              \
        {                                       \
          return _ ## _x;                       \
        }                                       \
        _t& type::_x()                          \
        {                                       \
          return _ ## _x;                       \
        }
#define ACCESS(x) ACCESST(bool,x)

      ACCESS (ignore_properties)

      ACCESS (warning_error)
      ACCESS (warning_all)
      ACCESS (warning_overwrite_function_name_as)
      ACCESS (warning_overwrite_template_name_as)
      ACCESS (warning_shadow_struct)
      ACCESS (warning_shadow_function)
      ACCESS (warning_shadow_template)
      ACCESS (warning_shadow_specialize)
      ACCESS (warning_default_construction)
      ACCESS (warning_unused_field)
      ACCESS (warning_port_not_connected)
      ACCESS (warning_unexpected_element)
      ACCESS (warning_overwrite_function_name_trans)
      ACCESS (warning_property_overwritten)
      ACCESS (warning_type_map_duplicate)
      ACCESS (warning_type_get_duplicate)
      ACCESS (warning_independent_place)
      ACCESS (warning_independent_transition)
      ACCESS (warning_conflicting_port_types)
      ACCESS (warning_overwrite_file)
      ACCESS (warning_backup_file)
      ACCESS (warning_duplicate_external_function)
      ACCESS (warning_property_unknown)
      ACCESS (warning_inline_many_output_ports)
      ACCESS (warning_virtual_place_not_tunneled)
      ACCESS (warning_duplicate_template_parameter)
      ACCESS (warning_synthesize_anonymous_function)
      ACCESS (warning_struct_redefined)

      ACCESS (no_inline)
      ACCESS (synthesize_virtual_places)
      ACCESS (force_overwrite_file)
      ACCESS (do_file_backup)

      ACCESS (dependencies_add_phony_targets)
      ACCESS (dump_dependenciesD)

#undef ACCESS
#undef ACCESST

#define WARN(x)                                                         \
        void type::warn (warning::x const& w) const                     \
        {                                                               \
          generic_warn (w, _warning_ ## x, _option_W ## x );                  \
        }

      WARN (overwrite_function_name_as)
      WARN (overwrite_template_name_as)
      WARN (default_construction)
      WARN (unused_field)
      WARN (port_not_connected)
      WARN (unexpected_element)
      WARN (overwrite_function_name_trans)
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
      WARN (synthesize_anonymous_function)
      WARN (struct_redefined)

#undef WARN

      void
      type::check_for_include_loop (::boost::filesystem::path const& path) const
      {
        for ( auto pos (_in_progress.begin())
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

      void type::add_options (::boost::program_options::options_description& desc)
      {
#define TYPEDVAL(t,x)                                                   \
        ::boost::program_options::value<t>(&_ ## x)->default_value (_ ## x)
#define BOOLVAL(x)                              \
        TYPEDVAL (bool,x)->implicit_value(true)
#define STRINGVAL(x)                            \
        TYPEDVAL (std::string,x)
#define STRINGVECVAL(x)                                                 \
        ::boost::program_options::value<std::vector<std::string>> (&_ ## x)

        ::boost::program_options::options_description warnings ("Warnings");

        warnings.add_options()
          ( _option_Werror.c_str()
          , BOOLVAL (warning_error)
          , "cast warnings to errors"
          )
          ( _option_Wall.c_str()
          , BOOLVAL (warning_all)
          , "turn on all warnings"
          )
          ( _option_Woverwrite_function_name_as.c_str()
          , BOOLVAL (warning_overwrite_function_name_as)
          , "warn when overwriting a function name by 'as'"
          )
          ( _option_Woverwrite_template_name_as.c_str()
          , BOOLVAL (warning_overwrite_template_name_as)
          , "warn when overwriting a template name by 'as'"
          )
          ( _option_Wshadow_struct.c_str()
          , BOOLVAL (warning_shadow_struct)
          , "warn when shadowing a struct definition"
          )
          ( _option_Wshadow_function.c_str()
          , BOOLVAL (warning_shadow_function)
          , "warn when shadowing a function definition"
          )
          ( _option_Wshadow_template.c_str()
          , BOOLVAL (warning_shadow_template)
          , "warn when shadowing a template definition"
          )
          ( _option_Wshadow_specialize.c_str()
          , BOOLVAL (warning_shadow_specialize)
          , "warn when shadowing a specialization"
          )
          ( _option_Wdefault_construction.c_str()
          , BOOLVAL (warning_default_construction)
          , "warn when default construct (part of) tokens"
          )
          ( _option_Wunused_field.c_str()
          , BOOLVAL (warning_unused_field)
          , "warn when given fields in tokens are unused"
          )
          ( _option_Wport_not_connected.c_str()
          , BOOLVAL (warning_port_not_connected)
          , "warn when ports are not connected"
          )
          ( _option_Wunexpected_element.c_str()
          , BOOLVAL (warning_unexpected_element)
          , "warn when unexpected elements occur"
          )
          ( _option_Woverwrite_function_name_trans.c_str()
          , BOOLVAL (warning_overwrite_function_name_trans)
          , "warn when overwriting a function name with a transition name"
          )
          ( _option_Wproperty_overwritten.c_str()
          , BOOLVAL (warning_property_overwritten)
          , "warn when overwriting a property"
          )
          ( _option_Wtype_map_duplicate.c_str()
          , BOOLVAL (warning_type_map_duplicate)
          , "warn about duplicate type maps"
          )
          ( _option_Wtype_get_duplicate.c_str()
          , BOOLVAL (warning_type_get_duplicate)
          , "warn about duplicate type gets"
          )
          ( _option_Windependent_place.c_str()
          , BOOLVAL (warning_independent_place)
          , "warn when a place has no connection at all"
          )
          ( _option_Windependent_transition.c_str()
          , BOOLVAL (warning_independent_transition)
          , "warn when a transition has no connection at all"
          )
          ( _option_Wconflicting_port_types.c_str()
          , BOOLVAL (warning_conflicting_port_types)
          , "warn when in/out port has conflicting types"
          )
          ( _option_Woverwrite_file.c_str()
          , BOOLVAL (warning_overwrite_file)
          , "warn when a file is overwritten"
          )
          ( _option_Wbackup_file.c_str()
          , BOOLVAL (warning_backup_file)
          , "warn when make a backup"
          )
          ( _option_Wduplicate_external_function.c_str()
          , BOOLVAL (warning_duplicate_external_function)
          , "warn when an external function has multiple occurrences"
          )
          ( _option_Wproperty_unknown.c_str()
          , BOOLVAL (warning_property_unknown)
          , "warn when a property is unknown"
          )
          ( _option_Winline_many_output_ports.c_str()
          , BOOLVAL (warning_inline_many_output_ports)
          , "warn when a transition with more than one connected output"
            " port is inlined. This could lead to problems when the number"
            " of tokens is not the same on all output ports."
          )
          ( _option_Wvirtual_place_not_tunneled.c_str()
          , BOOLVAL (warning_virtual_place_not_tunneled)
          , "warn when a virtual place is not tunneled"
          )
          ( _option_Wduplicate_template_parameter.c_str()
          , BOOLVAL (warning_duplicate_template_parameter)
          , "warn when a template paramater is duplicated"
          )
          ( _option_Wsynthesize_anonymous_function.c_str()
          , BOOLVAL (warning_synthesize_anonymous_function)
          , "warn when a anonymous top level function is synthesized"
          )
          ( _option_Wstruct_redefined.c_str()
          , BOOLVAL (warning_struct_redefined)
          , "warn when a struct is visible via two paths"
          )
          ;

        ::boost::program_options::options_description
          generate ("Wrapper generation");

        generate.add_options()
          ( _option_gen_ldflags.c_str()
          , ::boost::program_options::value<gen_param_type>(&_gen_ldflags)
          , "ldflags for generated makefile"
          )
          ( _option_gen_cxxflags.c_str()
          , ::boost::program_options::value<gen_param_type>(&_gen_cxxflags)
          , "cxxflags for generated makefile"
          )
          ( _option_path_to_cpp.c_str()
          , STRINGVAL (path_to_cpp)->implicit_value ("gen")
          , "path for cpp output, empty for no cpp output"
          )
          ( _option_path_prefixes_to_strip.c_str()
          , STRINGVECVAL (path_prefixes_to_strip)
          , "path prefix to strip in #line pragmas"
          )
          ;

        ::boost::program_options::options_description xml ("XML mirroring");

        xml.add_options()
          ( _option_dump_xml_file.c_str()
          , STRINGVAL (dump_xml_file)->implicit_value("/dev/stdout")
          , "file to dump the folded and pretty xml, empty for no dump"
          )
          ;

        ::boost::program_options::options_description
          depend ("Dependency generation");

        depend.add_options()
          ( _option_dump_dependencies.c_str()
          , STRINGVAL (dump_dependencies)->implicit_value("/dev/stdout")
          , "file to dump the dependencies as Make target, empty for no dump"
          " (also as -MM -MF -MG)"
          )
          ( _option_dump_dependenciesD.c_str()
          , BOOLVAL (dump_dependenciesD)
          , "dump the dependencies as Make target to <input>.d"
          " (also as -MD -MMD)"
          )
          ( _option_dependencies_target.c_str()
          , STRINGVECVAL (dependencies_target)
          , "set the target in the dependency file (also as -MT)"
          )
          ( _option_dependencies_target_quoted.c_str()
          , STRINGVECVAL (dependencies_target_quoted)
          , "like -MT but quote special characters (also as -MQ)"
          )
          ( _option_dependencies_add_phony_targets.c_str()
          , BOOLVAL (dependencies_add_phony_targets)
          , "add phony targets for all dependencies (also as -MP)"
          )
          ( _option_list_dependencies.c_str()
          , STRINGVAL (list_dependencies)->implicit_value("/dev/stdout")
          , "file to list the dependencies, empty for no list"
          )
          ;

        ::boost::program_options::options_description net ("Network handling");

        net.add_options()
          ( _option_ignore_properties.c_str()
          , BOOLVAL (ignore_properties)
          , "when set to true, no properties are parsed"
          )
          ( _option_no_inline.c_str()
          , BOOLVAL (no_inline)
          , "if set, ignore the keyword inline"
          )
          ( _option_synthesize_virtual_places.c_str()
          , BOOLVAL (synthesize_virtual_places)
          , "if set, synthesize virtual places"
          )
          ;

        ::boost::program_options::options_description file ("File handling");

        file.add_options()
          ( _option_force_overwrite_file.c_str()
          , BOOLVAL (force_overwrite_file)
          , "force overwrite files"
          )
          ( _option_backup_extension.c_str()
          , STRINGVAL (backup_extension)
          , "backup extension"
          )
          ( _option_do_file_backup.c_str()
          , BOOLVAL (do_file_backup)
          , "make backup copies of files before overwriting"
          )
          ;

        desc.add_options()
          ( _option_search_path.c_str()
          , ::boost::program_options::value<search_path_type>(&_search_path)
          , "search path"
          )
          ;
#undef TYPEDVAL
#undef BOOLVAL
#undef STRINGVAL

        desc.add (net);

        desc.add (xml);
        desc.add (depend);
        desc.add (generate);
        desc.add (file);
        desc.add (warnings);
      }

      ::boost::filesystem::path type::strip_path_prefix
        (::boost::filesystem::path const& path) const
      {
        auto const try_strip
          ([&path] (::boost::filesystem::path const& to_strip)
           -> ::boost::optional<::boost::filesystem::path>
           {
             ::boost::filesystem::path::const_iterator
               pos_path (path.begin());
             ::boost::filesystem::path::const_iterator
               pos_to_strip (to_strip.begin());

             while (  pos_path != path.end()
                   && pos_to_strip != to_strip.end()
                   && (  *pos_to_strip == "."
                      || *pos_path == *pos_to_strip
                      )
                   )
             {
               if (*pos_to_strip != ".")
               {
                 ++pos_path;
               }
               ++pos_to_strip;
             }

             if (pos_to_strip == to_strip.end())
             {
               //! \note return ::boost::filesystem::path {pos_path,
               //! path.end()} does not compile, \todo why?
               ::boost::filesystem::path stripped;

               for (; pos_path != path.end(); ++pos_path)
               {
                 stripped /= *pos_path;
               }

               return std::move (stripped);
             }

             return ::boost::none;
           }
          );

        for (::boost::filesystem::path to_strip : _path_prefixes_to_strip)
        {
          ::boost::optional<::boost::filesystem::path> const stripped
            (try_strip (to_strip));

          if (!!stripped)
          {
            return stripped.get();
          }
        }

        return path;
      }

      namespace
      {
        using pair_type = std::pair<std::string, std::string>;

        pair_type mk (std::string const& param)
        {
          return std::make_pair (param, std::string());
        }
      }

      pair_type reg_M (std::string const& s)
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
