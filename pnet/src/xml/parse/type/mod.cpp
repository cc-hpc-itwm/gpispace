// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/mod.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/link.hpp>
#include <xml/parse/util/valid_name.hpp>

#include <fhg/util/xml.hpp>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      module_type::module_type ( ID_CONS_PARAM(module)
                               , PARENT_CONS_PARAM(function)
                               , const util::position_type& pod
                               )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
      {
        _id_mapper->put (_id, *this);
      }

      module_type::module_type ( ID_CONS_PARAM(module)
                               , PARENT_CONS_PARAM(function)
                               , const util::position_type& pod
                               , const std::string& name
                               , const std::string& function
                               , const boost::optional<std::string>& port_return
                               , const std::list<std::string>& port_arg
                               , const boost::optional<std::string>& code
                               , const boost::optional<util::position_type>& pod_of_code
                               , const std::list<std::string>& cincludes
                               , const std::list<std::string>& ldflags
                               , const std::list<std::string>& cxxflags
                               , const std::list<link_type>& links
                               )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _name (name)
        , _function (function)
        , _port_return (port_return)
        , _port_arg (port_arg)
        , _code (code)
        , _position_of_definition_of_code (pod_of_code)
        , _cincludes (cincludes)
        , _ldflags (ldflags)
        , _cxxflags (cxxflags)
        , _links (links)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& module_type::name() const
      {
        return _name;
      }
      const std::string& module_type::function() const
      {
        return _function;
      }
      const boost::optional<std::string>& module_type::port_return() const
      {
        return _port_return;
      }
      const std::list<std::string>& module_type::port_arg() const
      {
        return _port_arg;
      }
      const boost::optional<std::string>& module_type::code() const
      {
        return _code;
      }
      const boost::optional<util::position_type>
        module_type::position_of_definition_of_code() const
      {
        return _position_of_definition_of_code;
      }
      const std::list<std::string>& module_type::cincludes() const
      {
        return _cincludes;
      }
      const std::list<std::string>& module_type::ldflags() const
      {
        return _ldflags;
      }
      const std::list<std::string>& module_type::cxxflags() const
      {
        return _cxxflags;
      }
      const std::list<link_type>& module_type::links() const
      {
        return _links;
      }

      bool module_type::operator == (const module_type& other) const
      {
        return _port_return == other._port_return
          && _port_arg == other._port_arg
          && _code == other._code
          && _cincludes == other._cincludes
          && _ldflags == other._ldflags
          && _cxxflags == other._cxxflags
          && _links == other._links
          ;
      }

      void module_type::sanity_check() const
      {
        assert (has_parent());

        const function_type& outer_function (*parent());
        if (_port_return)
        {
          if (!outer_function.is_known_port (*_port_return))
          {
            throw error::function_description_with_unknown_port
              ( "return"
              , *_port_return
              , name()
              , function()
              , outer_function.position_of_definition().path()
              );
          }
        }

        BOOST_FOREACH (const std::string& port, _port_arg)
        {
          if (!outer_function.is_known_port (port))
          {
            throw error::function_description_with_unknown_port
              ( "argument"
              , port
              , name()
              , function()
              , outer_function.position_of_definition().path()
              );
          }
        }
      }

      id::ref::module module_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return module_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
          , _name
          , _function
          , _port_return
          , _port_arg
          , _code
          , _position_of_definition_of_code
          , _cincludes
          , _ldflags
          , _cxxflags
          , _links
          ).make_reference_id();
      }

      std::size_t hash_value (const module_type& m)
      {
        boost::hash<std::string> hasher;
        return hasher (m.name());
      }

      namespace dump
      {
        std::string dump_fun (const module_type & m)
        {
          std::ostringstream s;

          if (m.port_return())
            {
              s << *m.port_return() << " ";
            }

          s << m.function();

          s << " (";

          bool first (true);

          BOOST_FOREACH (const std::string& arg, m.port_arg())
            {
              if (!first)
                {
                  s << ", ";
                  first = false;
                }

              s << arg;
            }

          s << ")";

          return s.str();
        }

        void dump (::fhg::util::xml::xmlstream & s, const module_type & m)
        {
          s.open ("module");
          s.attr ("name", m.name());
          s.attr ("function", dump_fun (m));

          BOOST_FOREACH (const std::string& inc, m.cincludes())
            {
              s.open ("cinclude");
              s.attr ("href", inc);
              s.close ();
            }

          BOOST_FOREACH (const std::string& flag, m.ldflags())
            {
              s.open ("ld");
              s.attr ("flag", flag);
              s.close ();
            }

          BOOST_FOREACH (const std::string& flag, m.cxxflags())
            {
              s.open ("cxx");
              s.attr ("flag", flag);
              s.close ();
            }

          BOOST_FOREACH (const link_type& link, m.links())
            {
              ::xml::parse::type::dump::dump (s, link);
            }

          if (m.code())
            {
              s.open ("code");
              s.content ("<![CDATA[" + *m.code() + "]]>");
              s.close ();
            }

          s.close ();
        }
      }
    }
  }
}
