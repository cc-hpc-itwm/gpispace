// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/types.hpp>
#include <xml/parse/type/template.hpp>

#include <boost/foreach.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      template_type::template_type
        ( const id::tmpl& id
        , const id::net& parent
        , const boost::filesystem::path& path
        , const fhg::util::maybe<std::string>& name
        , const names_type& template_parameter
        , const function_type& function
        )
          : _id (id)
          , _parent (parent)
          , _template_parameter (template_parameter)
          , _function (function)
          , _name (name)
          , _path (path)
      {}

      const id::tmpl&
      template_type::id() const
      {
        return _id;
      }

      const id::net&
      template_type::parent() const
      {
        return _parent;
      }

      bool
      template_type::is_same (const template_type& other) const
      {
        return id() == other.id() and parent() == other.parent();
      }

      const fhg::util::maybe<std::string>&
      template_type::name() const
      {
        return _name;
      }
      const std::string&
      template_type::name(const std::string& name)
      {
        return *(_name = name);
      }

      const template_type::names_type&
      template_type::template_parameter () const
      {
        return _template_parameter;
      }

      function_type& template_type::function()
      {
        return _function;
      }
      const function_type& template_type::function() const
      {
        return _function;
      }

      const boost::filesystem::path& template_type::path() const
      {
        return _path;
      }

      void
      template_type::distribute_function ( const state::type& state
                                         , const functions_type& functions
                                         , const templates_type& templates
                                         , const specializes_type& specializes
                                         )
      {
        function().distribute_function ( state
                                       , functions
                                       , templates
                                       , specializes
                                       );
      }


      void template_type::specialize
        ( const type_map_type & map
        , const type_get_type & get
        , const xml::parse::struct_t::set_type & known_structs
        , const state::type & state
        )
      {
        function().specialize (map, get, known_structs, state);
      }

      namespace dump
      {
        void dump (xml_util::xmlstream & s, const template_type & t)
        {
          s.open ("template");
          s.attr ("name", t.name());

          BOOST_FOREACH (const std::string& tn, t.template_parameter())
            {
              s.open ("template-parameter");
              s.attr ("type", tn);
              s.close ();
            }

          dump (s, t.function());

          s.close ();
        }
      } // namespace dump
    }
  }
}
