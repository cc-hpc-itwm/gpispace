// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TEMPLATE_HPP
#define _XML_PARSE_TYPE_TEMPLATE_HPP 1

#include <xml/parse/type/function.hpp>
#include <xml/parse/util/id_type.hpp>

#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/optional.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct template_type
      {
      private:
        typedef boost::unordered_set<std::string> names_type;

        id::tmpl _id;
        id::net _parent;
        names_type _template_parameter;
        boost::optional<function_type> _function;
        boost::optional<std::string> _name;
        boost::filesystem::path _path;

      public:
        template_type ( const id::tmpl& id
                      , const id::net& parent
                      , const boost::filesystem::path& path
                      , const std::string& name
                      );
        template_type ( const id::tmpl& id
                      , const id::net& parent
                      , const boost::filesystem::path& path
                      );

        const id::tmpl& id() const;
        const id::net& parent() const;
        bool is_same (const template_type& other) const;
        const boost::optional<std::string>& name() const;
        const names_type& template_parameter () const;
        void insert_template_parameter ( const std::string& tn
                                       , const state::type& state
                                       );
        const boost::optional<function_type>& function() const;
        const function_type& function (const function_type& function);
        const boost::filesystem::path& path() const;
      };

      namespace dump
      {
        inline void dump ( xml_util::xmlstream & s
                         , const template_type & f
                         , const state::type & state
                         );
      } // namespace dump
    }
  }
}

#endif
