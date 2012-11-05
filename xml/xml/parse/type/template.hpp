// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TEMPLATE_HPP
#define _XML_PARSE_TYPE_TEMPLATE_HPP 1

#include <xml/parse/id/generic.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>

#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>

#include <string>

#include <fhg/util/maybe.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct tmpl_type
      {
        ID_SIGNATURES(tmpl);
        PARENT_SIGNATURES(net);

      public:
        typedef boost::unordered_set<std::string> names_type;

      private:
        names_type _tmpl_parameter;
        id::ref::function _function;
        fhg::util::maybe<std::string> _name;
        boost::filesystem::path _path;

      public:
        tmpl_type ( ID_CONS_PARAM(tmpl)
                  , PARENT_CONS_PARAM(net)
                  , const boost::filesystem::path& path
                  , const fhg::util::maybe<std::string>& name
                  , const names_type& names
                  , const id::ref::function& function
                  );

        const fhg::util::maybe<std::string>& name() const;
        const std::string& name (const std::string& name);

        const names_type& tmpl_parameter () const;

        boost::optional<const function_type&> function() const;
        boost::optional<function_type&> function();

        const boost::filesystem::path& path() const;

        void specialize ( const type_map_type & map
                        , const type_get_type & get
                        , const xml::parse::structure_type::set_type & known_structs
                        , state::type & state
                        );

        boost::optional<const id::ref::function&>
        get_function (const std::string&) const;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const tmpl_type&);
      }
    }
  }
}

#endif
