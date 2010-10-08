// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_SIGNATURE_CPP_HPP
#define _WE_TYPE_SIGNATURE_CPP_HPP

#include <we/type/signature.hpp>

#include <we/type/literal/cpp.hpp>

#include <fhg/util/show.hpp>
#include <fhg/util/maybe.hpp>

#include <iostream>
#include <string>

namespace signature
{
  namespace cpp
  {
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
        cpp_generic (std::ostream & _s, const unsigned int _l = 0)
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
          level (l+1); s << "x." << fieldname << " = value::get<"
                         << literal::cpp::translate (t)
                         << "> (v_" << (l-1)
                         << ");"
                         << std::endl
                         ;

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

              level (l+2); s << "const value::type & v_" << l << " "
                             << "(value::get_level (\"" << field->first << "\""
                             << ", v_" << (l-1) << "));"
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
                       << " = " << "x." << fieldname << ";"
                       << std::endl;

          return s;
        }

        std::ostream & operator () (const structured_t & map) const
        {
          level (l); s << "{" << std::endl;

          level (l+1); s << "value::structured_t v_" << l << ";"
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
                             << std::endl;
            }

          level (l); s << "}" << std::endl;

          return s;
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
          return s << literal::cpp::translate (t);
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
                           )
    {
      boost::apply_visitor (visitor::cpp_struct (n, os), s.desc());

      os << ";" << std::endl;

      os << n << " from_value (const value::type & v_0)" << std::endl;

      os << "{" << std::endl;
      os << "  " << n << " x;" << std::endl;

      boost::apply_visitor (visitor::cpp_from_value (os), s.desc());

      os << "  return x;" << std::endl;
      os << "}" << std::endl;

      os << "value::type to_value (const " << n << " & x)" << std::endl;

      boost::apply_visitor (visitor::cpp_to_value (os), s.desc());
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

      public:
        cpp_show ( std::ostream & _s
                 , const std::string & _field_local
                 , const std::string & _field_global
                 , const unsigned int _l = 0
                 )
          : cpp_generic (_s, _l)
          , field_local (_field_local)
          , field_global (_field_global)
        {}

        std::ostream & operator () (const literal::type_name_t &) const
        {
          s << "  s << \"";

          level (l);

          s << field_local << " = \" << ";

          s << "literal::show (literal::type (x"
            << fhg::util::show (field_global) << ")) << std::endl;"
            << std::endl
            ;

          return s;
        }

        std::ostream & operator () (const structured_t & map) const
        {
          s << "  s << \"";

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
                           , l+1
                           )
                , field->second
                );
            }

          s << "  s << \"";

          level (l);

          s << "}\" << std::endl;" << std::endl;

          return s;
        }
      };
    }

    inline void cpp_show ( std::ostream & os
                         , const type & s
                         , const std::string & n
                         )
    {
      os << "inline std::ostream & operator << (std::ostream & s, const "
         << n << " & x)" << std::endl;
      os << "{" << std::endl;

      boost::apply_visitor (visitor::cpp_show (os, n, ""), s.desc());

      os << std::endl << "  return s;" << std::endl;

      os << "}" << std::endl;
    }

    inline void cpp_show (std::ostream & os, const type & s)
    {
      cpp_show (os, s, s.nice());
    }

    // ********************************************************************* //

    inline void cpp_header ( std::ostream & os
                           , const type & s
                           , const std::string & n
                           )
    {
      os << "// automatically generated" << std::endl;
      os << "// generator by <mirko.rahn@itwm.fraunhofer.de>" << std::endl;
      os << std::endl;
      os << "#ifndef _SIG_CPP_HEADER_" << n << std::endl;
      os << "#define _SIG_CPP_HEADER_" << n << " 1 " << std::endl;
      os << std::endl;
      os << "// needed types" << std::endl;
      os << "#include <we/type/control.hpp>" << std::endl;
      os << "#include <we/type/bitsetofint.hpp>" << std::endl;
      os << "#include <string>" << std::endl;
      os << std::endl;
      os << "// for the connection to value::type" << std::endl;
      os << "#include <we/type/value.hpp>" << std::endl;
      //      os << "#include <we/type/value/show.hpp>" << std::endl;
      os << "#include <we/type/value/get.hpp>" << std::endl;
      os << std::endl;

      cpp_struct (os, s, n);

      os << std::endl;

#if 0
      os << "// for the operator <<" << std::endl;
      os << "#include <we/type/literal.hpp>" << std::endl;
      os << "#include <we/type/literal/show.hpp>" << std::endl;
      os << "#include <iostream>" << std::endl;
      os << std::endl;

      cpp_show (os, s, n);

      os << std::endl;
#endif

      os << "#endif" << std::endl;
    }

    inline void cpp_header (std::ostream & os, const type & s)
    {
      cpp_header (os, s, s.nice());
    }
  }
}

#endif
