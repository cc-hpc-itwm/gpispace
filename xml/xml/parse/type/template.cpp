// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

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
        , const boost::optional<std::string>& name
        , const names_type& tmpl_parameter
        , const id::ref::function& function
        , const boost::filesystem::path& path
        )
          : ID_INITIALIZE()
          , PARENT_INITIALIZE()
          , _name (name)
          , _tmpl_parameter (tmpl_parameter)
          , _function (function)
          , _path (path)
      {
        _id_mapper->put (_id, *this);
      }

      const boost::optional<std::string>& tmpl_type::name() const
      {
        return _name;
      }
      const std::string& tmpl_type::name_impl (const std::string& name)
      {
        return *(_name = name);
      }
      const std::string& tmpl_type::name(const std::string& name)
      {
        if (has_parent())
        {
          parent()->rename (make_reference_id(), name);
          return *_name;
        }

        return name_impl (name);
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

      void tmpl_type::specialize
        ( const type_map_type & map
        , const type_get_type & get
        , const xml::parse::structure_type::set_type & known_structs
        , state::type & state
        )
      {
        function().get_ref().specialize (map, get, known_structs, state);
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

      const tmpl_type::unique_key_type& tmpl_type::unique_key() const
      {
        //! \note Anonymous templates can't be stored in unique, thus
        //! just indirect.
        return *name();
      }


      id::ref::tmpl tmpl_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return tmpl_type
          ( new_id
          , new_mapper
          , parent
          , _name
          , _tmpl_parameter
          , _function.get().clone (function_type::make_parent (new_id), mapper)
          , _path
          ).make_reference_id();
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
