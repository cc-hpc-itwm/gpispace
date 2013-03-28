// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_MOD_HPP
#define _XML_PARSE_TYPE_MOD_HPP

#include <xml/parse/id/generic.hpp>

#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/link.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>

#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>
#include <list>

#include <boost/filesystem.hpp>
#include <boost/unordered/unordered_map_fwd.hpp>
#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct module_type : with_position_of_definition
      {
        ID_SIGNATURES(module);
        PARENT_SIGNATURES(function);

      public:
        typedef std::list<std::string> port_args_type;
        typedef std::list<std::string> cincludes_type;
        typedef std::list<std::string> flags_type;
        typedef std::list<link_type> links_type;

        module_type ( ID_CONS_PARAM(module)
                    , PARENT_CONS_PARAM(function)
                    , const util::position_type&
                    );
        module_type ( ID_CONS_PARAM(module)
                    , PARENT_CONS_PARAM(function)
                    , const util::position_type&
                    , const std::string & _name
                    , const std::string & _function
                    );
        module_type ( ID_CONS_PARAM(module)
                    , PARENT_CONS_PARAM(function)
                    , const util::position_type&
                    , const std::string& name
                    , const std::string& function
                    , const boost::optional<std::string>& port_return
                    , const port_args_type& port_arg
                    , const boost::optional<std::string>& code
                    , const boost::optional<util::position_type>& pod_of_code
                    , const cincludes_type& cincludes
                    , const flags_type& ldflags
                    , const flags_type& cxxflags
                    , const links_type& links
                    );

        const std::string& name() const;

        bool operator == (const module_type& other) const;

        void sanity_check() const;

        id::ref::module clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

        friend std::size_t hash_value (const module_type&);

      private:
        std::string _name;

      public:
        //! \todo All these should be private with accessors
        std::string function;
        boost::optional<std::string> port_return;
        port_args_type port_arg;

        boost::optional<std::string> code;
        boost::optional<util::position_type> position_of_definition_of_code;
        cincludes_type cincludes;
        flags_type ldflags;
        flags_type cxxflags;
        links_type links;
      };

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
