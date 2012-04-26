// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_MOD_HPP
#define _XML_PARSE_TYPE_MOD_HPP

#include <string>
#include <iostream>

#include <fhg/util/maybe.hpp>
#include <fhg/util/parse/position.hpp>

#include <xml/parse/util/valid_name.hpp>
#include <xml/parse/error.hpp>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <list>

#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::list<std::string> port_args_type;
      typedef std::list<std::string> cincludes_type;
      typedef std::list<std::string> flags_type;

      struct mod_type
      {
      public:
        std::string name;
        std::string function;
        boost::optional<std::string> port_return;
        port_args_type port_arg;

        boost::optional<std::string> code;
        cincludes_type cincludes;
        flags_type ldflags;
        flags_type cxxflags;

        // ***************************************************************** //

        template<typename Fun>
        void sanity_check (const Fun & fun) const
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

        // ***************************************************************** //

        mod_type () {}
        mod_type ( const std::string & _name
                 , const std::string & _function
                 , const boost::filesystem::path & path
                 )
          : name (_name)
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
        }
      };

      namespace dump
      {
        inline std::string dump_fun (const mod_type & m)
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

        inline void dump (xml_util::xmlstream & s, const mod_type & m)
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

#endif
