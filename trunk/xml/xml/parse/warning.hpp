// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_WARNING_HPP
#define _XML_PARSE_WARNING_HPP

#include <stdexcept>
#include <string>
#include <sstream>

#include <we/type/signature.hpp>
#include <we/type/property.hpp>
#include <we/type/value.hpp>

#include <fhg/util/join.hpp>

namespace xml
{
  namespace parse
  {
    namespace warning
    {
      class generic : public std::runtime_error
      {
      public:
        generic (const std::string & msg)
          : std::runtime_error ("WARNING: " + msg)
        {}
      };

      // ******************************************************************* //

      class unexpected_element : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const std::string & pre
                         , const boost::filesystem::path & path
                         )
        {
          std::ostringstream s;

          s << pre
            << ": unexpected element with name " << util::quote(name)
            << " in " << path
            ;

          return s.str();
        }

      public:
        unexpected_element ( const std::string & name
                           , const std::string & pre
                           , const boost::filesystem::path & path
                           )
          : generic (nice (name, pre, path))
        {}
      };

      // ******************************************************************* //

      class overwrite_function_name_as : public generic
      {
      private:
        std::string nice ( const std::string & old_name
                         , const std::string & new_name
                         , const boost::filesystem::path & path
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
        overwrite_function_name_as ( const std::string & old_name
                                   , const std::string & new_name
                                   , const boost::filesystem::path & path
                                   )
          : generic (nice (old_name, new_name, path))
        {}
      };

      // ******************************************************************* //

      class overwrite_template_name_as : public generic
      {
      private:
        std::string nice ( const std::string & old_name
                         , const std::string & new_name
                         , const boost::filesystem::path & path
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
        overwrite_template_name_as ( const std::string & old_name
                                   , const std::string & new_name
                                   , const boost::filesystem::path & path
                                   )
          : generic (nice (old_name, new_name, path))
        {}
      };

      // ******************************************************************* //

      template<typename T>
      class struct_shadowed : public generic
      {
      private:
        std::string nice (const T & early, const T & late) const
        {
          std::ostringstream s;

          s << "struct with name " << late.name
            << " in " << late.path
            << " shadows definition from " << early.path
            ;

          return s.str();
        }

      public:
        struct_shadowed (const T & early, const T & late)
          : generic (nice (early, late))
        {}
      };

      // ******************************************************************* //

      class default_construction : public generic
      {
      private:
        std::string nice ( const std::string & place
                         , const std::string & field
                         , const boost::filesystem::path & path
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
        default_construction ( const std::string & place
                             , const std::string & field
                             , const boost::filesystem::path & path
                             )
          : generic (nice (place, field, path))
        {}
      };

      // ******************************************************************* //

      class unused_field : public generic
      {
      private:
        std::string nice ( const std::string & place
                         , const std::string & field
                         , const boost::filesystem::path & path
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
        unused_field ( const std::string & place
                             , const std::string & field
                             , const boost::filesystem::path & path
                             )
          : generic (nice (place, field, path))
        {}
      };

      // ******************************************************************* //

      class port_not_connected : public generic
      {
      private:
        std::string nice ( const std::string & direction
                         , const std::string & port
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << direction << "-port " << port
            << " not connected"
            << " in " << path
            ;

          return s.str();
        }
      public:
        port_not_connected ( const std::string & direction
                           , const std::string & port
                           , const boost::filesystem::path & path
                           )
          : generic (nice (direction, port, path))
        {}
      };

      // ******************************************************************* //

      class overwrite_function_name_trans : public generic
      {
      private:
        std::string nice ( const std::string & fun
                         , const boost::filesystem::path & funpath
                         , const std::string & trans
                         , const boost::filesystem::path & transpath
                         )
        {
          std::ostringstream s;

          s << "old function name " << fun
            << " defined in " << funpath
            << " overwritten with transition name " << trans
            << " in " << transpath
            ;

          return s.str();
        }
      public:
        overwrite_function_name_trans
        ( const std::string & fun
        , const boost::filesystem::path & funpath
        , const std::string & trans
        , const boost::filesystem::path & transpath
        )
          : generic (nice (fun, funpath, trans, transpath))
        {}
      };

      // ******************************************************************* //

      class overwrite_function_internal_trans : public generic
      {
      private:
        std::string nice ( const std::string & trans
                         , const boost::filesystem::path & path
                         )
        {
          std::ostringstream s;

          s << " transition " << trans
            << " in " << path
            << " overwrites the internal tag of the contained function"
            ;

          return s.str();
        }
      public:
        overwrite_function_internal_trans ( const std::string & trans
                                          , const boost::filesystem::path & path
                                          )
          : generic (nice (trans, path))
        {}
      };

      // ******************************************************************* //

      class property_overwritten : public generic
      {
      private:
        std::string nice ( const we::type::property::path_type & key
                         , const we::type::property::mapped_type & old_val
                         , const we::type::property::value_type & new_val
                         , const boost::filesystem::path & path
                         )
        {
          std::ostringstream s;

          s << "property " << fhg::util::join (key, ".")
            << " value " << old_val
            << " overwritten by value " << new_val
            << " in " << path
            ;

          return s.str();
        }
      public:
        property_overwritten ( const we::type::property::path_type & key
                             , const we::type::property::mapped_type & old_val
                             , const we::type::property::value_type & new_val
                             , const boost::filesystem::path & path
                             )
          : generic (nice (key, old_val, new_val, path))
        {}
      };

      // ******************************************************************* //

      class type_map_duplicate : public generic
      {
      private:
        std::string nice ( const std::string & replace
                         , const std::string & with
                         , const boost::filesystem::path & path
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
        type_map_duplicate ( const std::string & replace
                           , const std::string & with
                           , const boost::filesystem::path & path
                           )
          : generic (nice (replace, with, path))
        {}
      };

      // ******************************************************************* //

      class type_get_duplicate : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const boost::filesystem::path & path
                         )
        {
          std::ostringstream s;

          s << "type get duplicate, type " << name
            << " in " << path
            ;

          return s.str();
        }
      public:
        type_get_duplicate ( const std::string & name
                           , const boost::filesystem::path & path
                           )
          : generic (nice (name, path))
        {}
      };

      // ******************************************************************* //

      class overwrite_context : public generic
      {
      private:
        std::string nice ( const std::string & key
                         , const std::string & val
                         , const value::type & old_val
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "old value " << old_val
            << " of property " << key
            << " overwritten by " << val
            << " in " << path
            ;

          return s.str();
        }

      public:
        overwrite_context ( const std::string & key
                          , const std::string & val
                          , const value::type & old_val
                          , const boost::filesystem::path & path
                          )
          : generic (nice (key, val, old_val, path))
        {}
      };

      // ******************************************************************* //

      class independent_place : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const boost::filesystem::path & path
                         )
        {
          std::ostringstream s;

          s << "the place " << name << " has no connection at all"
            << " in " << path
            ;

          return s.str();
        }
      public:
        independent_place ( const std::string & name
                          , const boost::filesystem::path & path
                          )
          : generic (nice (name, path))
        {}
      };

      // ******************************************************************* //

      class independent_transition : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const boost::filesystem::path & path
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
        independent_transition ( const std::string & name
                               , const boost::filesystem::path & path
                               )
          : generic (nice (name, path))
        {}
      };

      // ******************************************************************* //

      class conflicting_port_types : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const std::string & port
                         , const std::string & type_in
                         , const std::string & type_out
                         , const boost::filesystem::path & path
                         )
        {
          std::ostringstream s;

          s << "the port " << port << " of the transition " << name
            << " has the type " << type_in << " as input port"
            << " and the type " << type_out << " as output port"
            << " in " << path
            ;

          return s.str();
        }
      public:
        conflicting_port_types ( const std::string & name
                               , const std::string & port
                               , const std::string & type_in
                               , const std::string & type_out
                               , const boost::filesystem::path & path
                               )
          : generic (nice (name, port, type_in, type_out, path))
        {}
      };

      // ******************************************************************* //

      class overwrite_file : public generic
      {
      private:
        std::string nice (const boost::filesystem::path & file) const
        {
          std::ostringstream s;

          s << "overwrite the file " << file;

          return s.str();
        }

      public:
        overwrite_file (const boost::filesystem::path & file)
          : generic (nice (file))
        {}
      };

      // ******************************************************************* //

      class duplicate_external_function : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const std::string & mod
                         , const boost::filesystem::path & file
                         ) const
        {
          std::ostringstream s;

          s << "the external function " << name
            << " in module " << mod
            << " has multiple occurrences"
            << " in " << file
            ;

          return s.str();
        }
      public:
        duplicate_external_function ( const std::string & name
                                    , const std::string & mod
                                    , const boost::filesystem::path & file
                                    )
          : generic (nice (name, mod, file))
        {}
      };
    }
  }
}

#endif
