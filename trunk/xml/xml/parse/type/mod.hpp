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

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::vector<std::string> port_arg_vec_type;

      struct mod_type
      {
      public:
        std::string name;
        std::string function;
        fhg::util::maybe<std::string> port_return;
        port_arg_vec_type port_arg;

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

      std::ostream & operator << (std::ostream & s, const mod_type & m)
      {
        return s << "mod ("
                 << "mod = " << m.name
                 << ", function = " << m.function
                 << ", port_return = " << m.port_return
                 << ", port_arg = " << fhg::util::show ( m.port_arg.begin()
                                                       , m.port_arg.end()
                                                       )
                 << ")"
          ;
      }
    }
  }
}

#endif
