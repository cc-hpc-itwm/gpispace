// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_WARNING_HPP
#define _XML_PARSE_WARNING_HPP

#include <stdexcept>
#include <string>
#include <sstream>

#include <we/type/signature.hpp>
#include <we/type/property.hpp>

#include <xml/parse/util/join.hpp>

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

          s << "property " << util::join (key, ".")
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
    }
  }
}

#endif
