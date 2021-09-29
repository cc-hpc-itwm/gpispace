// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <xml/parse/state.fwd.hpp>

#include <xml/parse/type/require.hpp>
#include <xml/parse/warning.hpp>
#include <xml/parse/util/position.hpp>
#include <xml/parse/rapidxml/types.hpp>

#include <we/type/property.hpp>

#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace xml
{
  namespace parse
  {
    namespace state
    {
      typedef std::vector<std::string> gen_param_type;

      struct type
      {
      private:
        typedef std::list<boost::filesystem::path> in_progress_type;
        typedef std::vector<std::string> search_path_type;
        typedef std::map< const char*
                        , util::position_type
                        , std::greater<const char*>
                        > position_by_pointer_type;
        typedef std::list<position_by_pointer_type> in_progress_position_type;

      public:
        type();

        gen_param_type const& gen_ldflags() const;
        gen_param_type const& gen_cxxflags() const;
        gen_param_type& gen_ldflags();
        gen_param_type& gen_cxxflags();

        const ::xml::parse::type::requirements_type& requirements() const;

        void set_requirement
          (const ::xml::parse::type::require_key_type& key);

        we::type::property::path_type& prop_path();

        void interpret_property ( we::type::property::path_type const& path
                                , we::type::property::value_type const& value
                                );

        void set_input (boost::filesystem::path const& path);
        void set_input (std::string const& file);

        boost::filesystem::path file_in_progress() const;
        void set_in_progress_position (const char*);
        util::position_type position (const xml_node_type*) const;

        std::set<boost::filesystem::path> const& dependencies() const;

#define ACCESS(name)                     \
        std::string const& name() const; \
        std::string& name();

        ACCESS (path_to_cpp)
        ACCESS (dump_xml_file)
        ACCESS (dump_dependencies)
        ACCESS (list_dependencies)
        ACCESS (backup_extension)

#undef ACCESS

#define ACCESS(name) std::vector<std::string> const& name() const;

        ACCESS (dependencies_target)
        ACCESS (dependencies_target_quoted)

#undef ACCESS

#define ACCESST(_t,_x)                            \
          _t const& _x() const;                   \
        _t& _x();
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

#define WARN(x) void warn (warning::x const& w) const;

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

        template<typename T>
        T generic_parse ( std::function<T (std::istream&, type&)> parse
                        , std::istream& s
                        )
        {
          const T x (parse (s, *this));

          if (_in_progress_position.empty())
          {
            throw std::runtime_error ("no start position given");
          }

          _in_progress_position.pop_back();

          return x;
        }

        template<typename T>
        T generic_parse ( std::function<T (std::istream&, type&)> parse
                        , boost::filesystem::path const& path
                        )
        {
          _in_progress.push_back (path);

          std::ifstream stream (path.string().c_str());
          if (!stream)
          {
            throw std::runtime_error ("unable to open file " + path.string());
          }

          const T x (generic_parse<T> (parse, stream));

          _in_progress.pop_back();

          return x;
        }

        void check_for_include_loop (boost::filesystem::path const& path) const;

        template<typename T>
        T generic_include ( std::function<T (std::istream&, type&)> parse
                          , std::string const& file
                          )
        {
          const boost::filesystem::path path (expand (file));

          _dependencies.insert (path);

          check_for_include_loop (path);

          return generic_parse<T> (parse, path);
        }

        void add_options (boost::program_options::options_description& desc);

        boost::filesystem::path strip_path_prefix
          (boost::filesystem::path const&) const;

      private:
        ::xml::parse::type::requirements_type _requirements;
        search_path_type _search_path;
        gen_param_type _gen_ldflags;
        gen_param_type _gen_cxxflags;
        in_progress_type _in_progress;
        mutable in_progress_position_type _in_progress_position;
        std::set<boost::filesystem::path> _dependencies;
        we::type::property::path_type _prop_path;
        bool _ignore_properties;
        bool _warning_error;
        bool _warning_all;
        bool _warning_overwrite_function_name_as;
        bool _warning_overwrite_template_name_as;
        bool _warning_shadow_struct;
        bool _warning_shadow_function;
        bool _warning_shadow_template;
        bool _warning_shadow_specialize;
        bool _warning_default_construction;
        bool _warning_unused_field;
        bool _warning_port_not_connected;
        bool _warning_unexpected_element;
        bool _warning_overwrite_function_name_trans;
        bool _warning_property_overwritten;
        bool _warning_type_map_duplicate;
        bool _warning_type_get_duplicate;
        bool _warning_independent_place;
        bool _warning_independent_transition;
        bool _warning_conflicting_port_types;
        bool _warning_overwrite_file;
        bool _warning_backup_file;
        bool _warning_duplicate_external_function;
        bool _warning_property_unknown;
        bool _warning_inline_many_output_ports;
        bool _warning_virtual_place_not_tunneled;
        bool _warning_duplicate_template_parameter;
        bool _warning_synthesize_anonymous_function;
        bool _warning_struct_redefined;

        std::string _dump_xml_file;
        std::string _dump_dependencies;
        std::string _list_dependencies;
        bool _dump_dependenciesD;
        std::vector<std::string> _dependencies_target;
        std::vector<std::string> _dependencies_target_quoted;
        bool _dependencies_add_phony_targets;
        bool _no_inline;
        bool _synthesize_virtual_places;
        bool _force_overwrite_file;
        std::string _backup_extension;
        bool _do_file_backup;

        std::string _path_to_cpp;

        std::vector<std::string> _path_prefixes_to_strip;

        std::string _option_search_path;
        std::string _option_gen_ldflags;
        std::string _option_gen_cxxflags;
        std::string _option_ignore_properties;
        std::string _option_Werror;
        std::string _option_Wall;
        std::string _option_Woverwrite_function_name_as;
        std::string _option_Woverwrite_template_name_as;
        std::string _option_Wshadow_struct;
        std::string _option_Wshadow_function;
        std::string _option_Wshadow_template;
        std::string _option_Wshadow_specialize;
        std::string _option_Wdefault_construction;
        std::string _option_Wunused_field;
        std::string _option_Wport_not_connected;
        std::string _option_Wunexpected_element;
        std::string _option_Woverwrite_function_name_trans;
        std::string _option_Wproperty_overwritten;
        std::string _option_Wtype_map_duplicate;
        std::string _option_Wtype_get_duplicate;
        std::string _option_Windependent_place;
        std::string _option_Windependent_transition;
        std::string _option_Wconflicting_port_types;
        std::string _option_Woverwrite_file;
        std::string _option_Wbackup_file;
        std::string _option_Wduplicate_external_function;
        std::string _option_Wproperty_unknown;
        std::string _option_Winline_many_output_ports;
        std::string _option_Wvirtual_place_not_tunneled;
        std::string _option_Wduplicate_template_parameter;
        std::string _option_Wsynthesize_anonymous_function;
        std::string _option_Wstruct_redefined;

        std::string _option_dump_xml_file;
        std::string _option_dump_dependencies;
        std::string _option_list_dependencies;
        std::string _option_dump_dependenciesD;
        std::string _option_dependencies_target;
        std::string _option_dependencies_target_quoted;
        std::string _option_dependencies_add_phony_targets;
        std::string _option_no_inline;
        std::string _option_synthesize_virtual_places;
        std::string _option_force_overwrite_file;
        std::string _option_backup_extension;
        std::string _option_do_file_backup;

        std::string _option_path_to_cpp;
        std::string _option_path_prefixes_to_strip;

        template<typename W>
        void generic_warn ( W const& w
                          , bool const& active
                          , std::string const& flag
                          ) const;

        boost::filesystem::path expand (std::string const& file) const;
      };

      std::pair<std::string, std::string> reg_M (std::string const& s);
    }
  }
}
