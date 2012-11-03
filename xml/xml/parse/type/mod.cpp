// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/mod.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/util/valid_name.hpp>

#include <fhg/util/xml.hpp>

#include <boost/foreach.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::list<std::string> port_args_type;
      typedef std::list<std::string> cincludes_type;
      typedef std::list<std::string> flags_type;
      typedef std::list<std::string> links_type;

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
                               , const std::string & _name
                               , const std::string & _function
                               , const boost::filesystem::path & path
                               )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , name (_name)
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
        fhg::util::parse::position pos (k, begin, _function.end());

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

      void module_type::sanity_check (const function_type & fun) const
      {
        if (port_return)
        {
          if (!fun.is_known_port (*port_return))
          {
            throw error::function_description_with_unknown_port
              ( "return"
              , *port_return
              , name
              , function
              , fun.path
              );
          }
        }

        for ( port_args_type::const_iterator port (port_arg.begin())
            ; port != port_arg.end()
            ; ++port
            )
        {
          if (!fun.is_known_port (*port))
          {
            throw error::function_description_with_unknown_port
              ( "argument"
              , *port
              , name
              , function
              , fun.path
              );
          }
        }
      }

      std::size_t hash_value (const module_type& m)
      {
        boost::hash<std::string> hasher;
        return hasher (m.name);
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

          for ( port_args_type::const_iterator arg (m.port_arg.begin())
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
          s.attr ("name", m.name);
          s.attr ("function", dump_fun (m));

          for ( cincludes_type::const_iterator inc (m.cincludes.begin())
              ; inc != m.cincludes.end()
              ; ++inc
              )
            {
              s.open ("cinclude");
              s.attr ("href", *inc);
              s.close ();
            }

          BOOST_FOREACH (flags_type::value_type const& flag, m.ldflags)
            {
              s.open ("ld");
              s.attr ("flag", flag);
              s.close ();
            }

          BOOST_FOREACH (flags_type::value_type const& flag, m.cxxflags)
            {
              s.open ("cxx");
              s.attr ("flag", flag);
              s.close ();
            }

          BOOST_FOREACH (links_type::value_type const& link, m.links)
            {
              s.open ("link");
              s.attr ("href", link);
              s.close ();
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
