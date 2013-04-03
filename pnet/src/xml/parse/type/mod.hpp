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

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct module_type : with_position_of_definition
      {
        ID_SIGNATURES (module);
        PARENT_SIGNATURES (function);

      public:
        module_type ( ID_CONS_PARAM (module)
                    , PARENT_CONS_PARAM (function)
                    , const util::position_type&
                    );
        module_type ( ID_CONS_PARAM (module)
                    , PARENT_CONS_PARAM (function)
                    , const util::position_type&
                    , const std::string& name
                    , const std::string& function
                    , const boost::optional<std::string>& port_return
                    , const std::list<std::string>& port_arg
                    , const boost::optional<std::string>& code
                    , const boost::optional<util::position_type>& pod_of_code
                    , const std::list<std::string>& cincludes
                    , const std::list<std::string>& ldflags
                    , const std::list<std::string>& cxxflags
                    , const std::list<link_type>& links
                    );

        const std::string& name() const;
        const std::string& function() const;
        const boost::optional<std::string>& port_return() const;
        const std::list<std::string>& port_arg() const;
        const boost::optional<std::string>& code() const;
        const boost::optional<util::position_type>
          position_of_definition_of_code() const;
        const std::list<std::string>& cincludes() const;
        const std::list<std::string>& ldflags() const;
        const std::list<std::string>& cxxflags() const;
        const std::list<link_type>& links() const;

        bool operator== (const module_type&) const;

        void sanity_check() const;

        id::ref::module clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

        friend std::size_t hash_value (const module_type&);

      private:
        std::string _name;
        std::string _function;
        boost::optional<std::string> _port_return;
        std::list<std::string> _port_arg;
        boost::optional<std::string> _code;
        boost::optional<util::position_type> _position_of_definition_of_code;
        std::list<std::string> _cincludes;
        std::list<std::string> _ldflags;
        std::list<std::string> _cxxflags;
        std::list<link_type> _links;
      };

      std::size_t hash_value (const module_type&);

      namespace dump
      {
        std::string dump_fun (const module_type&);

        void dump (::fhg::util::xml::xmlstream&, const module_type&);
      }
    }
  }
}

#endif
