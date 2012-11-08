// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_MOD_HPP
#define _XML_PARSE_TYPE_MOD_HPP

#include <xml/parse/id/generic.hpp>

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
      struct module_type
      {
        ID_SIGNATURES(module);
        PARENT_SIGNATURES(function);

      public:
        typedef std::list<std::string> port_args_type;
        typedef std::list<std::string> cincludes_type;
        typedef std::list<std::string> flags_type;
        typedef std::list<std::string> links_type;

        module_type ( ID_CONS_PARAM(module)
                    , PARENT_CONS_PARAM(function)
                    );
        module_type ( ID_CONS_PARAM(module)
                    , PARENT_CONS_PARAM(function)
                    , const std::string & _name
                    , const std::string & _function
                    , const boost::filesystem::path & path
                    );
        module_type ( ID_CONS_PARAM(module)
                    , PARENT_CONS_PARAM(function)
                    , const std::string& name
                    , const std::string& function
                    , const fhg::util::maybe<std::string>& port_return
                    , const port_args_type& port_arg
                    , const fhg::util::maybe<std::string>& code
                    , const cincludes_type& cincludes
                    , const flags_type& ldflags
                    , const flags_type& cxxflags
                    , const links_type& links
                    , const boost::filesystem::path& path
                    );

        bool operator == (const module_type& other) const;

        void sanity_check (const function_type & fun) const;

        id::ref::module clone() const;

        friend std::size_t hash_value (const module_type&);

        //! \todo All these should be private with accessors
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
      };

      typedef boost::unordered_map<std::string, module_type> mc_by_function_type;
      typedef boost::unordered_map<std::string, mc_by_function_type> mcs_type;

      std::size_t hash_value (const module_type& m);

      namespace dump
      {
        std::string dump_fun (const module_type & m);

        void dump ( ::fhg::util::xml::xmlstream & s
                  , const module_type & m
                  );
      }
    }
  }
}

#endif
