// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_WARNING_HPP
#define _XML_PARSE_WARNING_HPP

#include <stdexcept>
#include <string>
#include <sstream>

#include <we/type/signature.hpp>

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

      class overwrite_function_name : public generic
      {
      public:
        overwrite_function_name ( const std::string & old_name
                                , const std::string & new_name
                                )
          : generic ( "old function name " + old_name
                    + " overwritten by new name " + new_name
                    )
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
    }
  }
}

#endif
