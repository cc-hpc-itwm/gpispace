// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/mod.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/link.hpp>
#include <xml/parse/util/valid_name.hpp>

#include <fhg/util/xml.hpp>

#include <boost/foreach.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      module_type::module_type ( ID_CONS_PARAM(module)
                               , PARENT_CONS_PARAM(function)
                               )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
      {
        _id_mapper->put (_id, *this);
      }

      module_type::module_type ( ID_CONS_PARAM(module)
                               , PARENT_CONS_PARAM(function)
                               , const std::string& name
                               , const std::string & _function
                               , const boost::filesystem::path & path
                               )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _name (name)
        , function ()
        , port_return ()
        , port_arg ()
      {
        // implement the grammar
        // S -> R F A
        // F -> valid_name
        // R -> eps | valid_name
        // A -> eps | '(' L ')' | '(' ')'
        // L -> valid_name | valid_name ',' L
        //
        // here R stands for the return port, F for the function
        // name and A for the list of argument ports

        std::size_t k (0);
        std::string::const_iterator begin (_function.begin());
        const std::string::const_iterator end (_function.end ());
        fhg::util::parse::position pos (k, begin, end);

        function = parse_name (pos);

        if (!pos.end())
        {
          if (*pos != '(')
          {
            port_return = function;
            function = parse_name (pos);
          }

          if (!pos.end())
          {
            if (*pos != '(')
            {
              throw error::parse_function::expected ( _name
                                                    , _function
                                                    , "("
                                                    , pos()
                                                    , path
                                                    );
            }

            ++pos;

            while (!pos.end() && *pos != ')')
            {
              port_arg.push_back (parse_name (pos));

              if (!pos.end() && *pos != ')')
              {
                if (*pos != ',')
                {
                  throw error::parse_function::expected ( _name
                                                        , _function
                                                        , ","
                                                        , pos()
                                                        , path
                                                        );
                }

                ++pos;
              }
            }

            if (pos.end() || *pos != ')')
            {
              throw error::parse_function::expected ( _name
                                                    , _function
                                                    , ")"
                                                    , pos()
                                                    , path
                                                    );
            }

            ++pos;
          }

          while (!pos.end() && isspace(*pos))
          {
            ++pos;
          }

          if (!pos.end())
          {
            throw error::parse_function::expected ( _name
                                                  , _function
                                                  , "<end of input>"
                                                  , pos()
                                                  , path
                                                  );
          }
        }

        _id_mapper->put (_id, *this);
      }

      module_type::module_type ( ID_CONS_PARAM(module)
                               , PARENT_CONS_PARAM(function)
                               , const std::string& name
                               , const std::string& function
                               , const boost::optional<std::string>& port_return
                               , const port_args_type& port_arg
                               , const boost::optional<std::string>& code
                               , const cincludes_type& cincludes
                               , const flags_type& ldflags
                               , const flags_type& cxxflags
                               , const links_type& links
                               , const boost::filesystem::path& path
                               )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _name (name)
        , function (function)
        , port_return (port_return)
        , port_arg (port_arg)
        , code (code)
        , cincludes (cincludes)
        , ldflags (ldflags)
        , cxxflags (cxxflags)
        , links (links)
        , path (path)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& module_type::name() const
      {
        return _name;
      }

      bool module_type::operator == (const module_type& other) const
      {
        return port_return == other.port_return
          && port_arg == other.port_arg
          && code == other.code
          && cincludes == other.cincludes
          && ldflags == other.ldflags
          && cxxflags == other.cxxflags
          && links == other.links
          ;
      }

      void module_type::sanity_check() const
      {
        assert (has_parent());

        const function_type& outer_function (*parent());
        if (port_return)
        {
          if (!outer_function.is_known_port (*port_return))
          {
            throw error::function_description_with_unknown_port
              ( "return"
              , *port_return
              , name()
              , function
              , outer_function.path
              );
          }
        }

        for ( port_args_type::const_iterator port (port_arg.begin())
            ; port != port_arg.end()
            ; ++port
            )
        {
          if (!outer_function.is_known_port (*port))
          {
            throw error::function_description_with_unknown_port
              ( "argument"
              , *port
              , name()
              , function
              , outer_function.path
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
          , _name
          , function
          , port_return
          , port_arg
          , code
          , cincludes
          , ldflags
          , cxxflags
          , links
          , path
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

          if (m.port_return)
            {
              s << *m.port_return << " ";
            }

          s << m.function;

          s << " (";

          bool first (true);

          for ( module_type::port_args_type::const_iterator arg (m.port_arg.begin())
              ; arg != m.port_arg.end()
              ; ++arg, first = false
              )
            {
              if (!first)
                {
                  s << ", ";
                }

              s << *arg;
            }

          s << ")";

          return s.str();
        }

        void dump (::fhg::util::xml::xmlstream & s, const module_type & m)
        {
          s.open ("module");
          s.attr ("name", m.name());
          s.attr ("function", dump_fun (m));

          for ( module_type::cincludes_type::const_iterator inc (m.cincludes.begin())
              ; inc != m.cincludes.end()
              ; ++inc
              )
            {
              s.open ("cinclude");
              s.attr ("href", *inc);
              s.close ();
            }

          BOOST_FOREACH (module_type::flags_type::value_type const& flag, m.ldflags)
            {
              s.open ("ld");
              s.attr ("flag", flag);
              s.close ();
            }

          BOOST_FOREACH (module_type::flags_type::value_type const& flag, m.cxxflags)
            {
              s.open ("cxx");
              s.attr ("flag", flag);
              s.close ();
            }

          BOOST_FOREACH (module_type::links_type::value_type const& link, m.links)
            {
              ::xml::parse::type::dump::dump (s, link);
            }

          if (m.code)
            {
              s.open ("code");
              s.content ("<![CDATA[" + *m.code + "]]>");
              s.close ();
            }

          s.close ();
        }
      }
    }
  }
}
