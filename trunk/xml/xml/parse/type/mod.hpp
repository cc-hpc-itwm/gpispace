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

#include <vector>

#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::vector<std::string> port_arg_vec_type;
      typedef std::vector<std::string> cinclude_list_type;
      typedef std::vector<std::string> link_list_type;

      struct mod_type
      {
      public:
        std::string name;
        std::string function;
        fhg::util::maybe<std::string> port_return;
        port_arg_vec_type port_arg;

        fhg::util::maybe<std::string> code;
        cinclude_list_type cincludes;
        link_list_type links;

        int level;

        // ***************************************************************** //

        template<typename Fun>
        void sanity_check (const Fun & fun) const
        {
          if (port_return.isJust())
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

          for ( port_arg_vec_type::const_iterator port (port_arg.begin())
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

        mod_type ( const std::string & _name
                 , const std::string & _function
                 , const boost::filesystem::path & path
                 , const int _level
                 )
          : name (_name)
          , function ()
          , port_return ()
          , port_arg ()
          , level (_level)
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

          if (m.port_return.isJust())
            {
              s << *m.port_return << " ";
            }

          s << m.function;

          s << " (";

          bool first (true);

          for ( port_arg_vec_type::const_iterator arg (m.port_arg.begin())
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

          for ( cinclude_list_type::const_iterator inc (m.cincludes.begin())
              ; inc != m.cincludes.end()
              ; ++inc
              )
            {
              s.open ("cinclude");
              s.attr ("href", *inc);
              s.close ();
            }

          for ( link_list_type::const_iterator link (m.links.begin())
              ; link != m.links.end()
              ; ++link
              )
            {
              s.open ("link");
              s.attr ("href", *link);
              s.close ();
            }

          if (m.code.isJust())
            {
              s.open ("code");
              s.content ("<![CDATA[" + *m.code + "]]>");
              s.close ();
            }

          s.close ();
        }
      }

      std::ostream & operator << (std::ostream & s, const mod_type & m)
      {
        s << level (m.level) << "mod (" << std::endl;

        s << level (m.level+1) << "mod = " << m.name << std::endl;
        s << level (m.level+1) << "function = " << m.function << std::endl;
        s << level (m.level+1) << "port_return = " << m.port_return << std::endl;
        s << level (m.level+1) << "port_arg = "
          << fhg::util::show (m.port_arg.begin(), m.port_arg.end()) << std::endl;

        s << level (m.level+1) << "cincludes = "
          << fhg::util::show (m.cincludes.begin(), m.cincludes.end()) << std::endl;
        s << level (m.level+1) << "links = "
          << fhg::util::show (m.links.begin(), m.links.end()) << std::endl;
        s << level (m.level+1) << "code = " << m.code << std::endl;

        return s << level (m.level) << ")";
      }
    }
  }
}

#endif
