// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/signature/complete.hpp>
#include <we2/type/signature/cpp.hpp>
#include <we2/type/signature/is_literal.hpp>
#include <we2/type/signature/name.hpp>
#include <we2/type/signature/traverse.hpp>
#include <we2/type/value/name.hpp>

#include <fhg/util/indenter.hpp>
#include <fhg/util/cpp/block.hpp>
#include <fhg/util/cpp/namespace.hpp>
#include <fhg/util/cpp/struct.hpp>
#include <fhg/util/cpp/include.hpp>

#include <boost/foreach.hpp>

#include <set>
#include <iostream>
#include <sstream>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace cpp
      {
        namespace
        {
          class printer
          {
          public:
            printer (std::ostream& os, fhg::util::indenter& indent)
              : _os (os)
              , _indent (indent)
            {}

          protected:
            std::ostream& _os;
            fhg::util::indenter& _indent;

            template<typename P>
            void traverse_fields
              (P p, const std::pair<std::string, structure_type>& s) const
            {
              BOOST_FOREACH (const field_type& f, s.second)
              {
                traverse (p, f);
              }
            }
          };

          template<typename F, typename State = int>
          class printer_for_field : public printer
          {
          public:
            printer_for_field ( std::ostream& os
                              , fhg::util::indenter& indent
                              , State state
                              )
              : printer (os, indent)
              , _state (state)
            {}
            printer_for_field ( std::ostream& os
                              , fhg::util::indenter& indent
                              )
              : printer (os, indent)
              , _state()
            {}
            void _struct (const std::pair<std::string, structure_type>& s) const
            {
              traverse_fields (*this, s);
            }
            void _field (const std::pair<std::string, std::string>& f) const
            {
              F()(_os, _indent, f.first, f.second, _state);
            }
            void _field_struct (const std::pair<std::string, structure_type>& s) const
            {
              F()(_os, _indent, s.first, s.first, _state);
            }
          private:
            State _state;
          };

          class field_decl
          {
          public:
            void operator() ( std::ostream& os
                            , fhg::util::indenter& indent
                            , const std::string& name
                            , const std::string& type
                            , const int
                            )
            {
              os << indent << complete (type) << " " << name <<  ";";
            }
          };

          typedef printer_for_field<field_decl> print_field_decl;

          class print_header : public printer
          {
          public:
            print_header (std::ostream& os, fhg::util::indenter& indent)
              : printer (os, indent)
            {}

            void _struct (const std::pair<std::string, structure_type>& s) const
            {
              namespace ns = fhg::util::cpp::ns;
              namespace structure = fhg::util::cpp::structure;

              _os << ns::open (_indent, s.first);

              traverse_fields (*this, s);

              _os << structure::open (_indent, "type");

              traverse (print_field_decl (_os, _indent), s);

              _os << std::endl;

              ctor_default();
              ctor_value();

              _os << std::endl;

              template_serialize (s);

              _os << structure::close (_indent);

              _os << std::endl;

              to_value();
              show();

              _os << ns::close (_indent);
            }
            void _field (const std::pair<std::string, std::string>&) const
            {}
            void _field_struct
              (const std::pair<std::string, structure_type>& s) const
            {
              traverse (*this, s);
            }

          private:
            void ctor_default() const
            {
              _os << _indent << "type();";
            }
            void ctor_value() const
            {
              _os << _indent
                  << "explicit type (const pnet::type::value::value_type&);";
            }
            void template_serialize
              (const std::pair<std::string, structure_type>& s) const
            {
              namespace block = fhg::util::cpp::block;

              _os << _indent << "template<typename Archive>"
                  << _indent
                  << "void serialize (Archive& ar, const unsigned int)"
                  << block::open (_indent);

              BOOST_FOREACH (const field_type& f, s.second)
              {
                _os << _indent
                    << "ar & BOOST_SERIALIZATION_NVP ("
                    << signature::name (f)
                    << ");";
              }

              _os << block::close (_indent);
            }
            void to_value() const
            {
              _os << _indent
                  << "pnet::type::value::value_type value (const type&);";
            }
            void show() const
            {
              _os << _indent
                  << "std::ostream& operator<< (std::ostream&, const type&);";
            }
          };
        }

        header::header (const structured_type& structured)
          : _structured (structured)
        {}
        std::ostream& header::operator() (std::ostream& os) const
        {
          fhg::util::indenter indent;

          os << fhg::util::cpp::include ("we2/type/value.hpp");
          os << fhg::util::cpp::include ("boost/serialization/nvp.hpp");
          os << fhg::util::cpp::include ("iosfwd");

          os << fhg::util::cpp::ns::open (indent, "pnetc");
          os << fhg::util::cpp::ns::open (indent, "type");

          traverse (print_header (os, indent), _structured);

          os << fhg::util::cpp::ns::close (indent);
          os << fhg::util::cpp::ns::close (indent);

          return os;
        }
        std::ostream& operator<< (std::ostream& os, const header& h)
        {
          return h (os);
        }

        namespace
        {
          class print_header_signature
            : public boost::static_visitor<std::ostream&>
          {
          public:
            print_header_signature (std::ostream& os)
              : _os (os)
            {}
            std::ostream& operator() (const std::string& s) const
            {
              return _os << "//" << s;
            }
            std::ostream& operator() (const structured_type& s) const
            {
              return _os << header (s);
            }
          private:
            std::ostream& _os;
          };
        }

        header_signature::header_signature (const signature_type& signature)
          : _signature (signature)
        {}
        std::ostream& header_signature::operator() (std::ostream& os) const
        {
          return boost::apply_visitor (print_header_signature (os), _signature);
        }
        std::ostream& operator<< (std::ostream& os, const header_signature& h)
        {
          return h (os);
        }


        namespace
        {
          class impl_ctor_default
          {
          public:
            void operator() ( std::ostream& os
                            , fhg::util::indenter& indent
                            , const std::string& name
                            , const std::string& type
                            , bool& first
                            ) const
            {
              os << fhg::util::deeper (indent)
                  << (first ? ':' : ',') << " " << name <<  "()";

              first = false;
            }
          };

          typedef printer_for_field<impl_ctor_default, bool&> print_ctor_default;

          class print_ctor_value : public printer
          {
          public:
            print_ctor_value ( std::ostream& os
                             , fhg::util::indenter& indent
                             , bool& first
                             )
              : printer (os, indent)
              , _first (first)
            {}
            void _struct (const std::pair<std::string, structure_type>& s) const
            {
              traverse_fields (*this, s);
            }
            void _field (const std::pair<std::string, std::string>& f) const
            {
              _os << fhg::util::deeper (_indent)
                  << (_first ? ':' : ',') << " " << f.first
                  <<  " ("
                  << "pnet::field";

              if (is_literal (f.second))
              {
                _os << "_as< " << complete (f.second) << " >";
              }

              _os << " ("
                  << "pnet::path (\"" << f.first << "\")"
                  << ", v"
                  << ", std::string(\"" << f.second << "\")"
                  << ")"
                  << ")";

              _first = false;
            }
            void _field_struct (const std::pair<std::string, structure_type>& s) const
            {
              _os << fhg::util::deeper (_indent)
                  << (_first ? ':' : ',') << " " << s.first
                  <<  " ("
                  << "pnet::field"
                  << " ("
                  << "pnet::path (\"" << s.first << "\")"
                  << ", v"
                  << ", pnet::signature_of ("
                  << s.first << "::value (" << s.first << "::type())"
                  << ")"
                  << ")"
                  << ")";

              _first = false;
            }
          private:
            bool& _first;
          };

          class impl_poke
          {
          public:
            void operator() ( std::ostream& os
                            , fhg::util::indenter& indent
                            , const std::string& name
                            , const std::string& type
                            , const int
                            ) const
            {
              os << indent
                 << "pnet::type::value::poke ("
                 << "\"" << name << "\""
                 << ", v, ";

              if (is_literal (type))
              {
                os << "x." << name;
              }
              else
              {
                os << type << "::value (" << "x." << name << ")";
              }

              os << ");";
            }
          };

          typedef printer_for_field<impl_poke> print_impl_poke;

          class print_impl : public printer
          {
          public:
            print_impl (std::ostream& os, fhg::util::indenter& indent)
              : printer (os, indent)
            {}

            void _struct (const std::pair<std::string, structure_type>& s) const
            {
              namespace ns = fhg::util::cpp::ns;

              _os << ns::open (_indent, s.first);

              traverse_fields (*this, s);

              ctor_default (s);
              ctor_value (s);
              from_value (s);
              show();

              _os << ns::close (_indent);
            }
            void _field (const std::pair<std::string, std::string>& f) const
            {}
            void _field_struct (const std::pair<std::string, structure_type>& s) const
            {
              traverse (*this, s);
            }
          private:
            void ctor_default
              (const std::pair<std::string, structure_type>& s) const
            {
              bool first (true);

              _os << _indent << "type::type()";

              traverse (print_ctor_default (_os, _indent, first), s);

              _os << _indent << "{}";
            }

            void ctor_value
              (const std::pair<std::string, structure_type>& s) const
            {
              bool first (true);

              _os << _indent
                  << "type::type (const pnet::type::value::value_type& v)";

              traverse (print_ctor_value (_os, _indent, first), s);

              _os << _indent << "{}";
            }

            void from_value
              (const std::pair<std::string, structure_type>& s) const
            {
              namespace block = fhg::util::cpp::block;

              _os << _indent
                  << "pnet::type::value::value_type value (const type& x)"
                  << block::open (_indent)
                  << _indent << "pnet::type::value::value_type v;";

              traverse (print_impl_poke (_os, _indent), s);

              _os << _indent << "return v;"
                  << block::close (_indent);
            }

            void show() const
            {
              namespace block = fhg::util::cpp::block;

              _os << _indent
                  << "std::ostream& operator<< (std::ostream& os, const type& x)"
                  << block::open (_indent)
                  << _indent
                  << "return os << pnet::type::value::show (value (x));"
                  << block::close (_indent)
                ;
            }
          };
        }

        impl::impl (const structured_type& structured)
          : _structured (structured)
        {}
        std::ostream& impl::operator() (std::ostream& os) const
        {
          fhg::util::indenter indent;

          os << fhg::util::cpp::include ("we2/field.hpp");
          os << fhg::util::cpp::include ("we2/signature_of.hpp");
          os << fhg::util::cpp::include ("we2/type/value/poke.hpp");
          os << fhg::util::cpp::include ("we2/type/value/show.hpp");
          os << fhg::util::cpp::include ("iostream");

          os << fhg::util::cpp::ns::open (indent, "pnetc");
          os << fhg::util::cpp::ns::open (indent, "type");

          traverse (print_impl (os, indent), _structured);

          os << fhg::util::cpp::ns::close (indent);
          os << fhg::util::cpp::ns::close (indent);

          return os;
        }
        std::ostream& operator<< (std::ostream& os, const impl& h)
        {
          return h (os);
        }

        namespace
        {
          class print_impl_signature
            : public boost::static_visitor<std::ostream&>
          {
          public:
            print_impl_signature (std::ostream& os)
              : _os (os)
            {}
            std::ostream& operator() (const std::string& s) const
            {
              return _os << "//" << s;
            }
            std::ostream& operator() (const structured_type& s) const
            {
              return _os << impl (s);
            }
          private:
            std::ostream& _os;
          };
        }

        impl_signature::impl_signature (const signature_type& signature)
          : _signature (signature)
        {}
        std::ostream& impl_signature::operator() (std::ostream& os) const
        {
          return boost::apply_visitor (print_impl_signature (os), _signature);
        }
        std::ostream& operator<< (std::ostream& os, const impl_signature& h)
        {
          return h (os);
        }
      }
    }
  }
}
