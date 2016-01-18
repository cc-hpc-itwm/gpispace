// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <xml/parse/id/generic.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <string>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <unordered_set>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct tmpl_type : with_position_of_definition
      {
        ID_SIGNATURES(tmpl);

      public:
        typedef std::string unique_key_type;
        typedef std::unordered_set<std::string> names_type;

        tmpl_type ( ID_CONS_PARAM(tmpl)
                  , const util::position_type&
                  , const boost::optional<std::string>& name
                  , const names_type& tmpl_parameter
                  , const id::ref::function& function
                  );

        const boost::optional<std::string>& name() const;
        const names_type& tmpl_parameter () const;

        const id::ref::function& function() const;

        void resolve_function_use_recursive
          (std::unordered_map<std::string, function_type const&> known);
        void resolve_types_recursive
          (std::unordered_map<std::string, pnet::type::signature::signature_type> known);

        const unique_key_type& unique_key() const;

        id::ref::tmpl clone
          ( const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        boost::optional<std::string> const _name;
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
