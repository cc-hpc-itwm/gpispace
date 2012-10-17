// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/types.hpp>
#include <xml/parse/type/template.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      template_type::template_type ( const id::tmpl& id
                                   , const id::net& parent
                                   , const boost::filesystem::path& path
                                   , const std::string& name
                                   )
        : _id (id)
        , _parent (parent)
        , _name (name)
        , _path (path)
      {}
      template_type::template_type ( const id::tmpl& id
                                   , const id::net& parent
                                   , const boost::filesystem::path& path
                                   )
        : _id (id)
        , _parent (parent)
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

      const boost::optional<std::string>&
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
      void
      template_type::insert_template_parameter ( const std::string& tn
                                               , const state::type& state
                                               )
      {
        //! \todo check and warn if already there

        _template_parameter.insert (tn);
      }

      const boost::optional<function_type>&
      template_type::function() const
      {
        return _function;
      }
      const function_type&
      template_type::function (const function_type& function)
      {
        return *(_function = function);
      }

      const boost::filesystem::path& template_type::path() const
      {
        return _path;
      }

      namespace dump
      {
        inline void dump ( xml_util::xmlstream & s
                         , const template_type & f
                         , const state::type & state
                         )
        {
        }
      } // namespace dump
    }
  }
}
