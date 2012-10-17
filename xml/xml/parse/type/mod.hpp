// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_MOD_HPP
#define _XML_PARSE_TYPE_MOD_HPP

#include <xml/parse/type/function.fwd.hpp>
#include <fhg/util/maybe.hpp>
#include <fhg/util/xml.fwd.hpp>

#include <string>
#include <list>

#include <boost/filesystem.hpp>
#include <boost/unordered/unordered_map_fwd.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::list<std::string> port_args_type;
      typedef std::list<std::string> cincludes_type;
      typedef std::list<std::string> flags_type;
      typedef std::list<std::string> links_type;

      struct mod_type
      {
      public:
        std::string name;
        std::string function;
        fhg::util::maybe<std::string> port_return;
        port_args_type port_arg;

        fhg::util::maybe<std::string> code;
        cincludes_type cincludes;
        flags_type ldflags;
        flags_type cxxflags;
        links_type links;
        boost::filesystem::path path;

        bool operator == (const mod_type& other) const;

        friend std::size_t hash_value (const mod_type&);

        void sanity_check (const function_type & fun) const;

        // ***************************************************************** //

        mod_type ();
        mod_type ( const std::string & _name
                 , const std::string & _function
                 , const boost::filesystem::path & path
                 );
      };

      typedef boost::unordered_map<std::string, mod_type> mc_by_function_type;
      typedef boost::unordered_map<std::string, mc_by_function_type> mcs_type;

      inline std::size_t hash_value (const mod_type& m);

      namespace dump
      {
        std::string dump_fun (const mod_type & m);

        void dump ( ::fhg::util::xml::xmlstream & s
                  , const mod_type & m
                  );
      }
    }
  }
}

#endif
