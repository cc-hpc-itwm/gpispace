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
        , const id::net& parent
        , const boost::filesystem::path& path
        , const fhg::util::maybe<std::string>& name
        , const names_type& tmpl_parameter
        , const id::ref::function& function
        )
          : ID_INITIALIZE()
          , _parent (parent)
          , _tmpl_parameter (tmpl_parameter)
          , _function (function)
          , _name (name)
          , _path (path)
      {
        _id_mapper->put (_id, *this);
      }

      const id::net&
      tmpl_type::parent() const
      {
        return _parent;
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

      boost::optional<function_type&> tmpl_type::function()
      {
        return id_mapper()->get_ref (_function);
      }
      boost::optional<const function_type&> tmpl_type::function() const
      {
        return id_mapper()->get (_function);
      }

      const boost::filesystem::path& tmpl_type::path() const
      {
        return _path;
      }

      boost::optional<function_type>
      tmpl_type::get_function (const std::string& name) const
      {
        std::cerr << "tmpl " << _name
                  << " asked for the function " << name
                  << std::endl;

        return boost::none;
      }

      void tmpl_type::specialize
        ( const type_map_type & map
        , const type_get_type & get
        , const xml::parse::structure_type::set_type & known_structs
        , state::type & state
        )
      {
        function()->specialize (map, get, known_structs, state);
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

          ::xml::parse::type::dump::dump (s, *t.function());

          s.close ();
        }
      } // namespace dump
    }
  }
}
