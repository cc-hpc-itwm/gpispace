// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/type/template.hpp>

#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/net.hpp>

#include <boost/foreach.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      tmpl_type::tmpl_type
        ( ID_CONS_PARAM(tmpl)
        , PARENT_CONS_PARAM(net)
        , const boost::filesystem::path& path
        , const fhg::util::maybe<std::string>& name
        , const names_type& tmpl_parameter
        , const id::ref::function& function
        )
          : ID_INITIALIZE()
          , PARENT_INITIALIZE()
          , _tmpl_parameter (tmpl_parameter)
          , _function (function)
          , _name (name)
          , _path (path)
      {
        _id_mapper->put (_id, *this);
      }

      const fhg::util::maybe<std::string>&
      tmpl_type::name() const
      {
        return _name;
      }
      const std::string&
      tmpl_type::name(const std::string& name)
      {
        return *(_name = name);
      }

      const tmpl_type::names_type&
      tmpl_type::tmpl_parameter () const
      {
        return _tmpl_parameter;
      }

      const id::ref::function& tmpl_type::function() const
      {
        return _function;
      }

      const boost::filesystem::path& tmpl_type::path() const
      {
        return _path;
      }

      boost::optional<const id::ref::function&>
      tmpl_type::get_function (const std::string& name) const
      {
        if (has_parent())
          {
            return parent()->get_function (name);
          }

        return boost::none;
      }

      void tmpl_type::specialize
        ( const type_map_type & map
        , const type_get_type & get
        , const xml::parse::structure_type::set_type & known_structs
        , state::type & state
        )
      {
        function().get_ref().specialize (map, get, known_structs, state);
      }

      namespace dump
      {
        void dump (xml_util::xmlstream & s, const tmpl_type & t)
        {
          s.open ("template");
          s.attr ("name", t.name());

          BOOST_FOREACH (const std::string& tn, t.tmpl_parameter())
            {
              s.open ("template-parameter");
              s.attr ("type", tn);
              s.close ();
            }

          ::xml::parse::type::dump::dump (s, t.function().get());

          s.close ();
        }
      } // namespace dump
    }
  }
}
