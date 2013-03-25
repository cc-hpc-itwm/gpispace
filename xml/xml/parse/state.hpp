// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_STATE_HPP
#define _XML_PARSE_STATE_HPP

#include <xml/parse/state.fwd.hpp>

#include <xml/parse/id/mapper.hpp>
#include <xml/parse/id/types.hpp>
#include <xml/parse/type/require.hpp>
#include <xml/parse/warning.hpp>
#include <xml/parse/util/position.hpp>
#include <xml/parse/rapidxml/types.hpp>

#include <we/type/bits/transition/optimize.hpp>
#include <we/type/property.hpp>

#include <fstream>
#include <list>
#include <set>
#include <map>
#include <string>
#include <vector>
#include <functional>

#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>
#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace state
    {
      typedef std::vector<std::string> gen_param_type;
      typedef std::vector<std::string> link_prefix_type;

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

        const gen_param_type& gen_ldflags() const;
        const gen_param_type& gen_cxxflags() const;
        gen_param_type& gen_ldflags();
        gen_param_type& gen_cxxflags();
        const link_prefix_type& link_prefix() const;
        const std::string& link_prefix_by_key (const std::string&) const;

        const ::xml::parse::type::requirements_type& requirements() const;

        void set_requirement
          ( const ::xml::parse::type::require_key_type& key
          , const bool& mandatory
          );

        we::type::property::path_type& prop_path();

        const we::type::optimize::options::type& options_optimize() const;

        void interpret_property ( const we::type::property::path_type& path
                                , const we::type::property::value_type& value
                                );

        void set_input (const boost::filesystem::path& path);
        void set_input (const std::string& file);

        boost::filesystem::path file_in_progress() const;
        void set_in_progress_position (const char*);
        util::position_type position (const xml_node_type*) const;

        const std::set<boost::filesystem::path>& dependencies() const;

#define ACCESS(name)                     \
        const std::string& name() const; \
        std::string& name();

        ACCESS (path_to_cpp)
        ACCESS (dump_xml_file)
        ACCESS (dump_dependencies)
        ACCESS (list_dependencies)
        ACCESS (backup_extension)

#undef ACCESS

#define ACCESS(name) const std::vector<std::string>& name() const;

        ACCESS (dependencies_target)
        ACCESS (dependencies_target_quoted)

#undef ACCESS

#define ACCESST(_t,_x)                            \
          const _t& _x() const;                   \
        _t& _x();
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

        ACCESS (no_inline)
        ACCESS (synthesize_virtual_places)
        ACCESS (force_overwrite_file)
        ACCESS (do_file_backup)

        ACCESS (dependencies_add_phony_targets)
        ACCESS (dump_dependenciesD)

#undef ACCESS
#undef ACCESST

#define WARN(x) void warn (const warning::x& w) const;

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
        WARN (struct_shadowed)

#undef WARN

        template<typename T>
        T generic_parse ( boost::function<T (std::istream&, type&)> parse
                        , const boost::filesystem::path& path
                        )
        {
          _in_progress.push_back (path);

          std::ifstream stream (path.string().c_str());

          const T x (parse (stream, *this));

          _in_progress.pop_back();

          if (_in_progress_position.empty())
          {
            throw std::runtime_error ("no start position given");
          }

          _in_progress_position.pop_back();

          return x;
        }

        template<typename T>
        T generic_parse ( boost::function<T (std::istream&, type&)> parse
                        , const std::string& file
                        )
        {
          return generic_parse<T> (parse, boost::filesystem::path (file));
        }

        void check_for_include_loop (const boost::filesystem::path& path) const;

        template<typename T>
        T generic_include ( boost::function<T (std::istream&, type&)> parse
                          , const std::string& file
                          )
        {
          const boost::filesystem::path path (expand (file));

          _dependencies.insert (path);

          check_for_include_loop (path);

          return generic_parse<T> (parse, path);
        };

        void add_options (boost::program_options::options_description& desc);

        const id::mapper* id_mapper() const;
        id::mapper* id_mapper();

      private:
        ::xml::parse::type::requirements_type _requirements;
        search_path_type _search_path;
        gen_param_type _gen_ldflags;
        gen_param_type _gen_cxxflags;
        in_progress_type _in_progress;
        mutable in_progress_position_type _in_progress_position;
        std::set<boost::filesystem::path> _dependencies;
        we::type::property::path_type _prop_path;
        we::type::optimize::options::type _options_optimize;
        bool _ignore_properties;
        bool _Werror;
        bool _Wall;
        bool _Woverwrite_function_name_as;
        bool _Woverwrite_template_name_as;
        bool _Wshadow_struct;
        bool _Wshadow_function;
        bool _Wshadow_template;
        bool _Wshadow_specialize;
        bool _Wdefault_construction;
        bool _Wunused_field;
        bool _Wport_not_connected;
        bool _Wunexpected_element;
        bool _Woverwrite_function_name_trans;
        bool _Woverwrite_function_internal_trans;
        bool _Wproperty_overwritten;
        bool _Wtype_map_duplicate;
        bool _Wtype_get_duplicate;
        bool _Windependent_place;
        bool _Windependent_transition;
        bool _Wconflicting_port_types;
        bool _Woverwrite_file;
        bool _Wbackup_file;
        bool _Wduplicate_external_function;
        bool _Wproperty_unknown;
        bool _Winline_many_output_ports;
        bool _Wvirtual_place_not_tunneled;
        bool _Wduplicate_template_parameter;

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

        link_prefix_type _link_prefix;

        mutable boost::unordered_map< std::string
                                    , std::string
                                    > _link_prefix_by_key;
        mutable bool _link_prefix_parsed;

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
        std::string _OWshadow_template;
        std::string _OWshadow_specialize;
        std::string _OWdefault_construction;
        std::string _OWunused_field;
        std::string _OWport_not_connected;
        std::string _OWunexpected_element;
        std::string _OWoverwrite_function_name_trans;
        std::string _OWoverwrite_function_internal_trans;
        std::string _OWproperty_overwritten;
        std::string _OWtype_map_duplicate;
        std::string _OWtype_get_duplicate;
        std::string _OWindependent_place;
        std::string _OWindependent_transition;
        std::string _OWconflicting_port_types;
        std::string _OWoverwrite_file;
        std::string _OWbackup_file;
        std::string _OWduplicate_external_function;
        std::string _OWproperty_unknown;
        std::string _OWinline_many_output_ports;
        std::string _OWvirtual_place_not_tunneled;
        std::string _OWduplicate_template_parameter;

        std::string _Odump_xml_file;
        std::string _Odump_dependencies;
        std::string _Olist_dependencies;
        std::string _Odump_dependenciesD;
        std::string _Odependencies_target;
        std::string _Odependencies_target_quoted;
        std::string _Odependencies_add_phony_targets;
        std::string _Ono_inline;
        std::string _Osynthesize_virtual_places;
        std::string _Oforce_overwrite_file;
        std::string _Obackup_extension;
        std::string _Odo_file_backup;

        std::string _Opath_to_cpp;
        std::string _Olink_prefix;

        id::mapper _id_mapper;

        template<typename W>
        void generic_warn ( const W& w
                          , const bool& active
                          , const std::string& flag
                          ) const;

        boost::filesystem::path expand (const std::string& file) const;
      };

      std::pair<std::string, std::string> reg_M (const std::string& s);
    }
  }
}

#endif
