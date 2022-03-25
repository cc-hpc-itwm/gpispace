// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <stdexcept>
#include <string>
#include <sstream>

#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/mod.fwd.hpp>
#include <xml/parse/type/port.fwd.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/transition.fwd.hpp>

#include <we/type/property.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/join.hpp>

#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

namespace xml
{
  namespace parse
  {
    namespace warning
    {
      class generic : public std::runtime_error
      {
      public:
        generic (std::string const& msg)
          : std::runtime_error ("WARNING: " + msg)
        {}
        generic (::boost::format const& bf)
          : std::runtime_error ("WARNING: " + bf.str())
        {}
      };

      // ******************************************************************* //

      class unexpected_element : public generic
      {
      private:
        std::string nice ( std::string const& name
                         , std::string const& pre
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << pre
            << ": unexpected element with name " << name
            << " in " << path
            ;

          return s.str();
        }

      public:
        unexpected_element ( std::string const& name
                           , std::string const& pre
                           , ::boost::filesystem::path const& path
                           )
          : generic (nice (name, pre, path))
        {}
      };

      // ******************************************************************* //

      class overwrite_function_name_as : public generic
      {
      private:
        std::string nice ( std::string const& old_name
                         , std::string const& new_name
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "old function name " << old_name
            << " overwritten by new name " << new_name
            << " in " << path
            ;

          return s.str();
        }

      public:
        overwrite_function_name_as ( std::string const& old_name
                                   , std::string const& new_name
                                   , ::boost::filesystem::path const& path
                                   )
          : generic (nice (old_name, new_name, path))
        {}
      };

      // ******************************************************************* //

      class overwrite_template_name_as : public generic
      {
      private:
        std::string nice ( std::string const& old_name
                         , std::string const& new_name
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "old template name " << old_name
            << " overwritten by new name " << new_name
            << " in " << path
            ;

          return s.str();
        }

      public:
        overwrite_template_name_as ( std::string const& old_name
                                   , std::string const& new_name
                                   , ::boost::filesystem::path const& path
                                   )
          : generic (nice (old_name, new_name, path))
        {}
      };

      // ******************************************************************* //

      class shadow_function : public generic
      {
      private:
        std::string nice ( ::boost::optional<std::string> const& name
                         , ::boost::filesystem::path const& path_early
                         , ::boost::filesystem::path const& path_late
                         ) const
        {
          std::ostringstream s;

          s << "function " << name << " shadowed "
            << "in " << path_late
            << ", first definition was in " << path_early
            ;

          return s.str();
        }

      public:
        shadow_function ( ::boost::optional<std::string> const& name
                        , ::boost::filesystem::path const& path_early
                        , ::boost::filesystem::path const& path_late
                        )
          : generic (nice (name, path_early, path_late))
        {}
      };

      // ******************************************************************* //

      class shadow_template : public generic
      {
      private:
        std::string nice ( ::boost::optional<std::string> const& name
                         , ::boost::filesystem::path const& path_early
                         , ::boost::filesystem::path const& path_late
                         ) const
        {
          std::ostringstream s;

          s << "template " << name << " shadowed "
            << "in " << path_late
            << ", first definition was in " << path_early
            ;

          return s.str();
        }

      public:
        shadow_template ( ::boost::optional<std::string> const& name
                        , ::boost::filesystem::path const& path_early
                        , ::boost::filesystem::path const& path_late
                        )
          : generic (nice (name, path_early, path_late))
        {}
      };

      // ******************************************************************* //

      class shadow_specialize : public generic
      {
      private:
        std::string nice ( ::boost::optional<std::string> const& name
                         , ::boost::filesystem::path const& path_early
                         , ::boost::filesystem::path const& path_late
                         ) const
        {
          std::ostringstream s;

          s << "specialization " << name << " shadowed "
            << "in " << path_late
            << ", first definition was in " << path_early
            ;

          return s.str();
        }

      public:
        shadow_specialize ( ::boost::optional<std::string> const& name
                          , ::boost::filesystem::path const& path_early
                          , ::boost::filesystem::path const& path_late
                          )
          : generic (nice (name, path_early, path_late))
        {}
      };

      // ******************************************************************* //

      class default_construction : public generic
      {
      private:
        std::string nice ( std::string const& place
                         , std::string const& field
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "default construction takes place for place " << place
            << " from " << path
            ;

          if (field != "")
            {
              s << " for field " << field;
            }

          return s.str();
        }

      public:
        default_construction ( std::string const& place
                             , std::string const& field
                             , ::boost::filesystem::path const& path
                             )
          : generic (nice (place, field, path))
        {}
      };

      // ******************************************************************* //

      class unused_field : public generic
      {
      private:
        std::string nice ( std::string const& place
                         , std::string const& field
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "for place " << place
            << " from " << path
            << " there is a field given with name " << field
            << " which is not used"
            ;

          return s.str();
        }

      public:
        unused_field ( std::string const& place
                             , std::string const& field
                             , ::boost::filesystem::path const& path
                             )
          : generic (nice (place, field, path))
        {}
      };

      // ******************************************************************* //

      class port_not_connected : public generic
      {
      public:
        port_not_connected (type::port_type const&, ::boost::filesystem::path const&);

      private:
        const ::boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class overwrite_function_name_trans : public generic
      {
      public:
        overwrite_function_name_trans ( type::transition_type const&
                                      , type::function_type const&
                                      );
      };

      // ******************************************************************* //

      class property_unknown : public generic
      {
      private:
        std::string nice ( we::type::property::path_type const& key
                         , we::type::property::value_type const& val
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "unknown property " << fhg::util::join (key, '.')
            << " value " << pnet::type::value::show (val)
            << " in " << path
            ;

          return s.str();
        }
      public:
        property_unknown ( we::type::property::path_type const& key
                         , we::type::property::value_type const& val
                         , ::boost::filesystem::path const& path
                         )
          : generic (nice (key, val, path))
        {}
      };

      // ******************************************************************* //

      class inline_many_output_ports : public generic
      {
      private:
        std::string nice ( std::string const& name
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "the inlined transition " << name
            << " has more than one connected output port"
            << " in " << path;

          return s.str();
        }
      public:
        inline_many_output_ports ( std::string const& name
                                 , ::boost::filesystem::path const& path
                                 )
          : generic (nice (name, path))
        {}
      };

      // ******************************************************************* //

      class property_overwritten : public generic
      {
      private:
        std::string nice ( we::type::property::path_type const& key
                         , pnet::type::value::value_type const& old_val
                         , we::type::property::value_type const& new_val
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "property " << fhg::util::join (key, '.')
            << " value " << pnet::type::value::show (old_val)
            << " overwritten by value " << pnet::type::value::show (new_val)
            << " in " << path
            ;

          return s.str();
        }
      public:
        property_overwritten ( we::type::property::path_type const& key
                             , pnet::type::value::value_type const& old_val
                             , we::type::property::value_type const& new_val
                             , ::boost::filesystem::path const& path
                             )
          : generic (nice (key, old_val, new_val, path))
        {}
      };

      // ******************************************************************* //

      class type_map_duplicate : public generic
      {
      private:
        std::string nice ( std::string const& replace
                         , std::string const& with
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "type map duplicate, type " << replace
            << " mapped to type " << with
            << " twice"
            << " in " << path
            ;

          return s.str();
        }
      public:
        type_map_duplicate ( std::string const& replace
                           , std::string const& with
                           , ::boost::filesystem::path const& path
                           )
          : generic (nice (replace, with, path))
        {}
      };

      // ******************************************************************* //

      class type_get_duplicate : public generic
      {
      private:
        std::string nice ( std::string const& name
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "type get duplicate, type " << name
            << " in " << path
            ;

          return s.str();
        }
      public:
        type_get_duplicate ( std::string const& name
                           , ::boost::filesystem::path const& path
                           )
          : generic (nice (name, path))
        {}
      };

      // ******************************************************************* //

      class independent_place : public generic
      {
      private:
        std::string nice ( std::string const& name
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "the place " << name << " has no connection at all"
            << " in " << path
            ;

          return s.str();
        }
      public:
        independent_place ( std::string const& name
                          , ::boost::filesystem::path const& path
                          )
          : generic (nice (name, path))
        {}
      };

      // ******************************************************************* //

      class independent_transition : public generic
      {
      private:
        std::string nice ( std::string const& name
                         , ::boost::filesystem::path const& path
                         )
        {
          std::ostringstream s;

          s << "the transition " << name << " has no connection at all"
            << " (or read connections only)"
            << " in " << path
            ;

          return s.str();
        }
      public:
        independent_transition ( std::string const& name
                               , ::boost::filesystem::path const& path
                               )
          : generic (nice (name, path))
        {}
      };

      // ******************************************************************* //

      class conflicting_port_types : public generic
      {
      public:
        conflicting_port_types ( type::transition_type const&
                               , type::port_type const& in
                               , type::port_type const& out
                               , ::boost::filesystem::path const&
                               );

      private:
        const ::boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class backup_file : public generic
      {
      private:
        std::string nice ( ::boost::filesystem::path const& from
                         , ::boost::filesystem::path const& to
                         ) const
        {
          std::ostringstream s;

          s << "make a backup of " << from << " in " << to;

          return s.str();
        }

      public:
        explicit backup_file ( ::boost::filesystem::path const& from
                             , ::boost::filesystem::path const& to
                             )
          : generic (nice (from, to))
        {}
      };

      // ******************************************************************* //

      class overwrite_file : public generic
      {
      private:
        std::string nice (::boost::filesystem::path const& file) const
        {
          std::ostringstream s;

          s << "overwrite the file " << file;

          return s.str();
        }

      public:
        overwrite_file (::boost::filesystem::path const& file)
          : generic (nice (file))
        {}
      };

      // ******************************************************************* //

      class duplicate_external_function : public generic
      {
      public:
        duplicate_external_function ( type::module_type const& mod
                                    , type::module_type const& old
                                    );
      };

      // ******************************************************************* //

      class virtual_place_not_tunneled : public generic
      {
      public:
        virtual_place_not_tunneled ( std::string const& name
                                   , ::boost::filesystem::path const& file
                                   )
          : generic ( ::boost::format ( "the virtual place %1%"
                                      " is not tunneled in %2%."
                                    )
                    % name
                    % file
                    )
        {}
      };

      // ******************************************************************* //

      class duplicate_template_parameter : public generic
      {
      public:
        duplicate_template_parameter ( ::boost::optional<std::string> name
                                     , std::string const& tn
                                     , ::boost::filesystem::path const& file
                                     )
          : generic ( ::boost::format ( "duplicate typename %2%"
                                      " in the definition of %1% in %3%"
                                    )
                    % (name ? *name : "<<noname>>")
                    % tn
                    % file
                    )
        {}
      };

      class synthesize_anonymous_function : public generic
      {
      public:
        synthesize_anonymous_function (type::function_type const&);
      };

      class struct_redefined : public generic
      {
      public:
        struct_redefined ( type::structure_type const& early
                         , type::structure_type const& late
                         );
      };
    }
  }
}
