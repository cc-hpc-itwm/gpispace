// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_SIGNATURE_CPP_HPP
#define _WE_TYPE_SIGNATURE_CPP_HPP

#include <we/type/signature.hpp>

#include <we/type/literal/cpp.hpp>

#include <fhg/util/show.hpp>
#include <fhg/util/maybe.hpp>

#include <fhg/util/cpp.hpp>

namespace cpp_util = fhg::util::cpp;

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>

namespace signature
{
  namespace cpp
  {
    inline void level (std::ostream & s, const unsigned int & j)
    {
      for (unsigned int i (0); i < j; ++i)
        {
          s << "  ";
        }
    }

    // ******************************************************************** //

    namespace visitor
    {
      // ****************************************************************** //

      class cpp_generic : public boost::static_visitor<std::ostream &>
      {
      protected:
        std::ostream & s;
        unsigned int l;

        void level (const unsigned int & j) const
        {
          for (unsigned int i (0); i < j; ++i)
            {
              s << "  ";
            }
        }

      public:
        cpp_generic (std::ostream & _s, const unsigned int & _l = 0)
          : s(_s), l (_l)
        {}
      };

      // ****************************************************************** //

      class cpp_from_value : public cpp_generic
      {
      private:
        signature::field_name_t fieldname;

        signature::field_name_t
        longer (const signature::field_name_t & sub) const
        {
          return fieldname.empty() ? sub : (fieldname + "." + sub);
        }

      public:
        cpp_from_value (std::ostream & _s, const unsigned int _l = 1)
          : cpp_generic (_s, _l), fieldname ()
        {}

        cpp_from_value ( const signature::field_name_t & _fieldname
                       , std::ostream & _s
                       , const unsigned int _l = 0
                       )
          : cpp_generic (_s, _l), fieldname (_fieldname)
        {}

        std::ostream & operator () (const literal::type_name_t & t) const
        {
          level (l+1);
          s << "x." << fieldname << " = ";



          if (literal::cpp::known (t))
            {
              s << cpp_util::access::make ("", "value", "get")
                << "< "
                << literal::cpp::translate (t)
                << " >";
            }
          else
            {
              s << cpp_util::access::make ( cpp_util::access::type()
                                          , t
                                          , "from_value"
                                          );
            }

          s << " (v_" << (l-1) << ");" << std::endl;

          return s;
        }

        std::ostream & operator () (const structured_t & map) const
        {
          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              level (l+1); s << "{" << std::endl;

              level (l+2);
              s << "const " << cpp_util::access::value_type()
                << " & v_" << l << " "
                << "(" << cpp_util::access::make ("", "value", "get_level")
                <<  "(\"" << field->first << "\"" << ", v_" << (l-1) << ")"
                << ");"
                << std::endl;

              boost::apply_visitor
                ( cpp_from_value (longer (field->first), s, l+1)
                , field->second
                );

              level (l+1); s << "}" << std::endl;
            }

          return s;
        }
      };
    }

    // ********************************************************************** //

    namespace visitor
    {
      class cpp_to_value : public cpp_generic
      {
      private:
        signature::field_name_t fieldname;
        signature::field_name_t levelname;

        signature::field_name_t
        longer (const signature::field_name_t & sub) const
        {
          return fieldname.empty() ? sub : (fieldname + "." + sub);
        }

      public:
        cpp_to_value (std::ostream & _s, const unsigned int _l = 0)
          : cpp_generic (_s, _l), fieldname ()
        {}

        cpp_to_value ( const signature::field_name_t & _fieldname
                     , const signature::field_name_t & _levelname
                     , std::ostream & _s
                     , const unsigned int _l = 0
                     )
          : cpp_generic (_s, _l), fieldname (_fieldname), levelname (_levelname)
        {}

        std::ostream & operator () (const literal::type_name_t & t) const
        {
          level (l); s << "v_" << (l-1) << "[\"" << levelname << "\"]"
                       << " = ";

          if (literal::cpp::known (t))
            {
              s << "x." << fieldname;
            }
          else
            {
              s << cpp_util::access::make ( cpp_util::access::type()
                                          , t, "to_value"
                                          )
                << "(x." << fieldname << ")"
                ;
            }

          s << ";" << std::endl;

          return s;
        }

        std::ostream & operator () (const structured_t & map) const
        {
          level (l); s << "{" << std::endl;

          level (l+1); s << cpp_util::access::make ("","value","structured_t")
                         << " v_" << l << ";"
                         << std::endl
                         ;

          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              boost::apply_visitor
                ( cpp_to_value (longer (field->first), field->first, s, l+1)
                , field->second
                );
            }

          if (fieldname.empty())
            {
              level (l+1); s << "return v_" << l << ";" << std::endl;
            }
          else
            {
              level (l+1); s << "v_" << (l-1) << "[\"" << levelname << "\"]"
                             << " = " << "v_" << l << ";"
                             << std::endl
                             ;
            }

          level (l); s << "} // to_value " << levelname << std::endl;

          return s;
        }
      };
    }

    // ********************************************************************** //

    namespace visitor
    {
      typedef boost::unordered_set<literal::type_name_t> seen_type;

      class cpp_includes : public boost::static_visitor<void>
      {
      private:
        std::ostream & os;
        seen_type & seen;
        const boost::filesystem::path & incpath;

      public:
        cpp_includes ( std::ostream & _os
                     , seen_type & _seen
                     , const boost::filesystem::path _incpath
                     )
          : os (_os), seen (_seen), incpath (_incpath)
        {}

        void operator () (const literal::type_name_t & t) const
        {
          if (!literal::cpp::known (t))
            {
              if (seen.find (t) == seen.end())
                {
                  const boost::filesystem::path file
                    (incpath / cpp_util::make::hpp (t));

                  cpp_util::include (os, file);

                  seen.insert (t);
                }
            }
        }

        void operator () (const structured_t & map) const
        {
          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              boost::apply_visitor ( cpp_includes (os, seen, incpath)
                                   , field->second
                                   );
            }
        }
      };
    }

    // ********************************************************************** //

    namespace visitor
    {
      class cpp_struct : public cpp_generic
      {
      private:
        fhg::util::maybe<std::string> name;

      public:
        cpp_struct ( const std::string & _name
                   , std::ostream & _s
                   , const unsigned int _l = 0
                   )
          : cpp_generic (_s, _l), name (_name)
        {}

        cpp_struct (std::ostream & _s, const unsigned int _l = 0)
          : cpp_generic (_s, _l), name ()
        {}

        std::ostream & operator () (const literal::type_name_t & t) const
        {
          if (literal::cpp::known (t))
            {
              s << literal::cpp::translate (t);
            }
          else
            {
              s << cpp_util::access::make (cpp_util::access::type(), t, t);
            }

          return s;
        }

        std::ostream & operator () (const structured_t & map) const
        {
          s << "struct ";

          if (name.isJust())
            {
              s << *name << " ";
            }

          s << "{" << std::endl;

          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              level(l+1);

              boost::apply_visitor (cpp_struct (s, l+1), field->second);

              s << " " << fhg::util::show (field->first) << ";" << std::endl;
            }

          level(l); s << "}";

          return s;
        }
      };
    }

    inline void cpp_struct ( std::ostream & os
                           , const type & s
                           , const std::string & n
                           , const unsigned int & l = 1
                           )
    {
      boost::apply_visitor (visitor::cpp_struct (n, os, l), s.desc());

      os << "; // struct " << n << std::endl;
      os << std::endl;

      level (os, l); os << "inline " << n << " from_value (const "
                        << cpp_util::access::value_type()
                        << "& v_" << (l-1) << ")"
                        << std::endl;
      level (os, l); os << "{" << std::endl;
      level (os, l); os << "  " << n << " x;" << std::endl;

      boost::apply_visitor (visitor::cpp_from_value (os, l), s.desc());

      level (os, l); os << "  return x;" << std::endl;
      level (os, l); os << "} // from_value " << std::endl;
      os << std::endl;

      level (os, l); os << "inline " << cpp_util::access::value_type()
                        << " to_value (const " << n << " & x)"
                        << std::endl;

      boost::apply_visitor (visitor::cpp_to_value (os, l), s.desc());
    }

    inline void cpp_struct (std::ostream & os, const type & s)
    {
      cpp_struct (os, s, s.nice());
    }

    // ********************************************************************** //

    namespace visitor
    {
      class cpp_show : public cpp_generic
      {
      private:
        const std::string & field_local;
        const std::string & field_global;

        const unsigned int l0;

      public:
        cpp_show ( std::ostream & _s
                 , const std::string & _field_local
                 , const std::string & _field_global
                 , const unsigned int & _l0 = 0
                 , const unsigned int & _l = 0
                 )
          : cpp_generic (_s, _l)
          , field_local (_field_local)
          , field_global (_field_global)
          , l0 (_l0)
        {}

        std::ostream & operator () (const literal::type_name_t & t) const
        {
          level (l0);

          s << "s << \"";

          level (l);

          s << field_local << " = \" << ";

          if (literal::cpp::known (t))
            {
              s << cpp_util::access::make ("","literal","show")
                << "(" << cpp_util::access::make ("", "literal", "type")
                << "(x" << fhg::util::show (field_global) << ")"
                << ") << std::endl"
                ;
            }
          else
            {
              s << "x" << fhg::util::show (field_global);
            }

          s << ";" << std::endl;

          return s;
        }

        std::ostream & operator () (const structured_t & map) const
        {
          level (l0);

          s << "s << \"";

          level (l);

          s << field_local << " = {\" << std::endl;" << std::endl;

          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              boost::apply_visitor
                ( cpp_show ( s
                           , field->first
                           , field_global + "." + field->first
                           , l0
                           , l+1
                           )
                , field->second
                );
            }

          level (l0);

          s << "s << \"";

          s << "}\" << std::endl;" << std::endl;

          return s;
        }
      };
    }

    inline void cpp_show ( std::ostream & os
                         , const type & s
                         , const std::string & n
                         , const unsigned int & l = 0
                         )
    {
      os << std::endl;

      level (os, l);

      os << "inline std::ostream & operator << (std::ostream & s, const "
         << n << " & x)"                          << std::endl;

      level (os, l);

      os << "{"                                   << std::endl;

      boost::apply_visitor (visitor::cpp_show (os, n, "", l + 1), s.desc());

      os                                          << std::endl;

      level (os, l);

      os << "  return s;"                         << std::endl;

      level (os, l);

      os << "}"                                   << std::endl;
    }

    inline void cpp_show ( std::ostream & os
                         , const type & s
                         , const unsigned int & l = 0
                         )
    {
      cpp_show (os, s, s.nice(), l);
    }

    // ********************************************************************* //

    inline void cpp_header ( std::ostream & os
                           , const type & s
                           , const std::string & n
                           , const boost::filesystem::path & defpath
                           , const boost::filesystem::path & incpath
                           )
    {
      cpp_util::header_gen_full (os);
      cpp_util::include_guard_begin (os, "PNETC_TYPE_" + n);

      cpp_util::include (os, "we/type/bitsetofint.hpp");
      cpp_util::include (os, "we/type/bytearray.hpp");
      cpp_util::include (os, "we/type/control.hpp");
      cpp_util::include (os, "we/type/value.hpp");
      cpp_util::include (os, "we/type/value/cpp/get.hpp");
      cpp_util::include (os, "string");

      // for operator <<
      cpp_util::include (os, "we/type/literal.hpp");
      cpp_util::include (os, "we/type/literal/show.hpp");
      cpp_util::include (os, "iostream");

      os << std::endl;

      visitor::seen_type seen;

      boost::apply_visitor ( visitor::cpp_includes (os, seen, incpath)
                           , s.desc()
                           );

      os << "namespace pnetc"                                    << std::endl;
      os << "{"                                                  << std::endl;
      os << "  namespace type"                                   << std::endl;
      os << "  {"                                                << std::endl;
      os                                                         << std::endl;
      os << "    // defined in " << defpath                      << std::endl;
      os << "    namespace " << n                                << std::endl;
      os << "    {"                                              << std::endl;

      os << "      "; cpp_struct (os, s, n, 3);

      os                                                         << std::endl;

      os << "      "; cpp_show (os, s, n, 3);

      os << "    } // namespace " << n                           << std::endl;
      os << "  } // namespace type"                              << std::endl;
      os << "} // namespace pnetc"                               << std::endl;
      os                                                         << std::endl;

      cpp_util::include_guard_end (os, "PNETC_TYPE_" + n);
    }

    inline void cpp_header (std::ostream & os, const type & s)
    {
      cpp_header ( os, s, s.nice()
                 , boost::filesystem::path ("<unknown>")
                 , boost::filesystem::path ("")
                 );
    }
  }
}

#endif
