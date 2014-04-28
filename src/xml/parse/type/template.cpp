// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <xml/parse/type/template.hpp>

#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/net.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      namespace
      {
        const id::ref::function& reparent ( const id::ref::function& function
                                          , const id::tmpl& id
                                          )
        {
          function.get_ref().parent (id);
          return function;
        }
      }

      tmpl_type::tmpl_type
        ( ID_CONS_PARAM(tmpl)
        , PARENT_CONS_PARAM(net)
        , const util::position_type& pod
        , const boost::optional<std::string>& name
        , const names_type& tmpl_parameter
        , const id::ref::function& function
        )
          : with_position_of_definition (pod)
          , ID_INITIALIZE()
          , PARENT_INITIALIZE()
          , _name (name)
          , _tmpl_parameter (tmpl_parameter)
          , _function (reparent (function, _id))
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

      boost::optional<const id::ref::function&>
      tmpl_type::get_function (const std::string& name) const
      {
        if (has_parent())
          {
            return parent()->get_function (name);
          }

        return boost::none;
      }

      boost::optional<pnet::type::signature::signature_type>
      tmpl_type::signature (const std::string& type) const
      {
        if (has_parent())
        {
          return parent()->signature (type);
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
          , _position_of_definition
          , _name
          , _tmpl_parameter
          , _function.get().clone (function_type::make_parent (new_id), mapper)
          ).make_reference_id();
      }

      namespace dump
      {
        void dump (fhg::util::xml::xmlstream & s, const tmpl_type & t)
        {
          s.open ("template");
          s.attr ("name", t.name());

          for (const std::string& tn : t.tmpl_parameter())
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
