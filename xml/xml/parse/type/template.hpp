// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TEMPLATE_HPP
#define _XML_PARSE_TYPE_TEMPLATE_HPP 1

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/id/types.hpp>

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
      struct template_type
      {
      public:
        typedef boost::unordered_set<std::string> names_type;

      private:

        id::tmpl _id;
        id::net _parent;
        names_type _template_parameter;
        function_type _function;
        fhg::util::maybe<std::string> _name;
        boost::filesystem::path _path;

      public:
        template_type ( const id::tmpl& id
                      , const id::net& parent
                      , const boost::filesystem::path& path
                      , const fhg::util::maybe<std::string>& name
                      , const names_type& names
                      , const function_type& function
                      );

        const id::tmpl& id() const;
        const id::net& parent() const;

        bool is_same (const template_type& other) const;

        const fhg::util::maybe<std::string>& name() const;
        const std::string& name (const std::string& name);

        const names_type& template_parameter () const;

        const function_type& function() const;
        function_type& function();

        const boost::filesystem::path& path() const;

        void distribute_function ( const state::type& state
                                 , const functions_type& functions
                                 , const templates_type& templates
                                 , const specializes_type& specializes
                                 );

        void specialize ( const type_map_type & map
                        , const type_get_type & get
                        , const xml::parse::struct_t::set_type & known_structs
                        , const state::type & state
                        );
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const template_type&);
      }
    }
  }
}

#endif
