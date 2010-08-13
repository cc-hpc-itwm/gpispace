// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_SIGNATURE_CPP_HPP
#define _WE_TYPE_SIGNATURE_CPP_HPP

#include <we/type/signature.hpp>

#include <we/type/literal/cpp.hpp>

#include <fhg/util/show.hpp>

#include <iostream>
#include <string>

namespace signature
{
  namespace cpp
  {
    namespace visitor
    {
      class cpp_typedef : public boost::static_visitor<std::ostream &>
      {
      private:
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
        cpp_typedef (std::ostream & _s, const unsigned int _l = 0)
          : s(_s), l (_l)
        {}

        std::ostream & operator () (const literal::type_name_t & t) const
        {
          return s << literal::cpp::translate (t);
        }

        std::ostream & operator () (const structured_t & map) const
        {
          s << "struct {" << std::endl;

          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              level(l+1);

              boost::apply_visitor (cpp_typedef (s, l+1), field->second);

              s << " " << fhg::util::show (field->first) << ";" << std::endl;
            }

          level(l); s << "}";

          return s;
        }
      };
    }

    inline void cpp_typedef ( std::ostream & os
                            , const type & s
                            , const std::string & n
                            )
    {
      os << "typedef ";

      boost::apply_visitor (visitor::cpp_typedef (os), s.desc());

      os << " " << n << ";" << std::endl;
    }

    inline void cpp_typedef (std::ostream & os, const type & s)
    {
      cpp_typedef (os, s, s.nice());
    }

    // ********************************************************************** //

    namespace visitor
    {
      class cpp_show : public boost::static_visitor<std::ostream &>
      {
      private:
        std::ostream & s;
        const std::string & field_local;
        const std::string & field_global;
        const unsigned int l;

        void level (const unsigned int & j) const
        {
          for (unsigned int i (0); i < j; ++i)
            {
              s << "  ";
            }
        }

      public:
        cpp_show ( std::ostream & _s
                 , const std::string & _field_local
                 , const std::string & _field_global
                 , const unsigned int _l = 0
                 )
          : s(_s)
          , field_local (_field_local)
          , field_global (_field_global)
          , l (_l)
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
      os << "std::ostream & operator << (std::ostream & s, const "
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
      os << "// for the operator <<" << std::endl;
      os << "#include <we/type/literal/show.hpp>" << std::endl;
      os << "#include <we/type/literal.hpp>" << std::endl;
      os << "#include <iostream>" << std::endl;
      os << std::endl;

      cpp_typedef (os, s, n);

      os << std::endl;

      cpp_show (os, s, n);

      os << std::endl;

      os << "#endif" << std::endl;
    }

    inline void cpp_header (std::ostream & os, const type & s)
    {
      cpp_header (os, s, s.nice());
    }
  }
}

#endif
