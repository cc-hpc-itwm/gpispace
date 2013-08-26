// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TEMPLATE_HPP
#define _XML_PARSE_TYPE_TEMPLATE_HPP 1

#include <xml/parse/id/generic.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <string>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_set.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct tmpl_type : with_position_of_definition
      {
        ID_SIGNATURES(tmpl);
        PARENT_SIGNATURES(net);

      public:
        typedef std::string unique_key_type;
        typedef boost::unordered_set<std::string> names_type;

        tmpl_type ( ID_CONS_PARAM(tmpl)
                  , PARENT_CONS_PARAM(net)
                  , const util::position_type&
                  , const boost::optional<std::string>& name
                  , const names_type& tmpl_parameter
                  , const id::ref::function& function
                  );

        const boost::optional<std::string>& name() const;
        const std::string& name (const std::string& name);

      private:
        friend struct net_type;
        const std::string& name_impl (const std::string& name);

      public:
        const names_type& tmpl_parameter () const;

        const id::ref::function& function() const;

        boost::optional<const id::ref::function&>
        get_function (const std::string&) const;

        boost::optional<pnet::type::signature::signature_type> signature (const std::string&) const;

        const unique_key_type& unique_key() const;

        id::ref::tmpl clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        boost::optional<std::string> _name;
        names_type _tmpl_parameter;
        id::ref::function _function;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const tmpl_type&);
      }
    }
  }
}

#endif
