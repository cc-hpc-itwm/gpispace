// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/complete.hpp>
#include <we/type/signature/cpp.hpp>
#include <we/type/signature/is_literal.hpp>
#include <we/type/signature/traverse.hpp>
#include <we/type/value/name.hpp>
#include <we/type/value/path/append.hpp>

#include <fhg/util/cpp/block.hpp>
#include <fhg/util/cpp/include.hpp>
#include <fhg/util/cpp/namespace.hpp>
#include <fhg/util/cpp/struct.hpp>
#include <fhg/util/indenter.hpp>

#include <util-generic/join.hpp>

#include <set>
#include <list>
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
              for (const field_type& f : s.second)
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
              os << indent << complete (type) << ' ' << name <<  ';';
            }
          };

          typedef printer_for_field<field_decl> print_field_decl;

          class field_param
          {
          public:
            void operator() ( std::ostream& os
                            , fhg::util::indenter& indent
                            , const std::string& name
                            , const std::string& type
                            , bool& first
                            )
            {
              os << fhg::util::deeper (indent)
                 << (first ? '(' : ',') << ' '
                 << complete (type) << " const& _" << name;

              first = false;
            }
          };

          typedef printer_for_field<field_param, bool&> print_field_param;

          class impl_ctor
          {
          public:
            void operator() ( std::ostream& os
                            , fhg::util::indenter& indent
                            , const std::string& name
                            , const std::string&
                            , bool& first
                            ) const
            {
              os << fhg::util::deeper (indent)
                 << (first ? ':' : ',') << ' ' << name << " (_" << name << ')';

              first = false;
            }
          };

          typedef printer_for_field<impl_ctor, bool&> print_ctor;

          class impl_ctor_default
          {
          public:
            void operator() ( std::ostream& os
                            , fhg::util::indenter& indent
                            , const std::string& name
                            , const std::string&
                            , bool& first
                            ) const
            {
              os << fhg::util::deeper (indent)
                 << (first ? ':' : ',') << ' ' << name <<  "()";

              first = false;
            }
          };

          typedef printer_for_field<impl_ctor_default, bool&> print_ctor_default;

          class impl_operator_less
          {
          public:
            void operator() ( std::ostream& os
                            , fhg::util::indenter&
                            , const std::string& name
                            , const std::string&
                            , std::list<std::string>& prefix
                            ) const
            {
              if (not prefix.empty())
              {
                os << " || ("
                   << "(this->" << prefix.back() << " == rhs." << prefix.back() << ")"
                   << " && ("
                  ;
              }

              os << "(this->" << name << " < rhs." << name << ")";

              prefix.push_back (name);
            }
          };

          typedef printer_for_field< impl_operator_less
                                   , std::list<std::string>&
                                   > print_field_operator_less;

          class impl_operator_eq
          {
          public:
            void operator() ( std::ostream& os
                            , fhg::util::indenter& indent
                            , const std::string& name
                            , const std::string&
                            , bool& first
                            ) const
            {
              os << fhg::util::deeper (indent)
                 << "&& (this->" << name << " == rhs." << name << ")";

              first = false;
            }
          };

          typedef printer_for_field< impl_operator_eq
                                   , bool&
                                   > print_field_operator_eq;

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

              _os << structure::open (_indent, s.first);

              traverse (print_field_decl (_os, _indent), s);

              ctor_default (s);

              if (!s.second.empty())
              {
                ctor (s);
              }
              operator_eq (s);
              operator_less (s);

              _os << structure::close (_indent);

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
            void operator_less
              (const std::pair<std::string, structure_type>& s) const
            {
              std::list<std::string> prefix;

              _os << _indent << "bool operator< ("
                  << s.first << " const& rhs) const"
                  << fhg::util::cpp::block::open (_indent)
                  << _indent << "return ";

              if (s.second.empty())
              {
                _os << "false";
              }
              else
              {
                traverse (print_field_operator_less (_os, _indent, prefix), s);

                if (not prefix.empty())
                {
                  _os << std::string (2 * (prefix.size() - 1), ')');
                }
              }

              _os << ";"
                  << fhg::util::cpp::block::close (_indent);
            }

            void operator_eq
              (const std::pair<std::string, structure_type>& s) const
            {
              bool first (true);

              _os << _indent << "bool operator== ("
                  << s.first << " const& rhs) const"
                  << fhg::util::cpp::block::open (_indent)
                  << _indent << "return true";

              traverse (print_field_operator_eq (_os, _indent, first), s);

              _os << fhg::util::deeper (_indent) << ";"
                  << fhg::util::cpp::block::close (_indent);
            }

            void ctor_default
              (const std::pair<std::string, structure_type>& s) const
            {
              bool first (true);

              _os << _indent << s.first << "()";

              traverse (print_ctor_default (_os, _indent, first), s);

              _os << _indent << "{}";
            }
            void ctor (const std::pair<std::string, structure_type>& s) const
            {
              bool first (true);

              _os << _indent << "explicit " << s.first;

              traverse (print_field_param (_os, _indent, first), s);

              _os << fhg::util::deeper (_indent) << ")";

              first = true;

              traverse (print_ctor (_os, _indent, first), s);

              _os << _indent << "{}";

            }
          };
        }

        header::header (const structured_type& structured)
          : _structured (structured)
        {}
        std::ostream& header::operator() (std::ostream& os) const
        {
          fhg::util::indenter indent;

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
          class print_header_op : public printer
          {
          public:
            print_header_op ( std::ostream& os
                            , fhg::util::indenter& indent
                            , std::list<std::list<std::string>>& tnames
                            , std::list<std::string>& tname
                            )
              : printer (os, indent)
              , _tnames (tnames)
              , _tname (tname)
            {}

            void _struct (const std::pair<std::string, structure_type>& s) const
            {
              using pnet::type::value::path::append;

              _os << fhg::util::cpp::ns::open (_indent, s.first);

              _tnames.push_front (append (append (_tname, s.first), s.first));

              traverse_fields ( print_header_op ( _os
                                                , _indent
                                                , _tnames
                                                , append (_tname, s.first)
                                                )
                              , s
                              );

              _os << _indent
                  << s.first
                  << " from_value (const pnet::type::value::value_type&);";

              _os << _indent
                  << "pnet::type::value::value_type to_value (const "
                  << s.first << "&);";

              _os << _indent
                  << "std::ostream& operator<< (std::ostream&, const "
                  << s.first
                  << "&);";

              _os << fhg::util::cpp::ns::close (_indent);
            }
            void _field (const std::pair<std::string, std::string>&) const
            {}
            void _field_struct
              (const std::pair<std::string, structure_type>& s) const
            {
              traverse (*this, s);
            }
          private:
            std::list<std::list<std::string>>& _tnames;
            std::list<std::string>& _tname;
          };
        }

        header_op::header_op (const structured_type& structured)
          : _structured (structured)
        {}
        std::ostream& header_op::operator() (std::ostream& os) const
        {
          fhg::util::indenter indent;

          os << fhg::util::cpp::include ("we/type/value.hpp");
          os << fhg::util::cpp::include ("we/type/value/to_value.hpp");
          os << fhg::util::cpp::include ("we/type/value/from_value.hpp");
          os << fhg::util::cpp::include ("iosfwd");

          os << fhg::util::cpp::ns::open (indent, "pnetc");
          os << fhg::util::cpp::ns::open (indent, "type");

          std::list<std::list<std::string>> tnames;

          {
            std::list<std::string> tname;
            tname.push_back ("pnetc");
            tname.push_back ("type");

            traverse (print_header_op (os, indent, tnames, tname), _structured);
          }

          os << fhg::util::cpp::ns::close (indent);
          os << fhg::util::cpp::ns::close (indent);

          os << fhg::util::cpp::ns::open (indent, "pnet");
          os << fhg::util::cpp::ns::open (indent, "type");
          os << fhg::util::cpp::ns::open (indent, "value");

          for (std::list<std::string> const& tname : tnames)
          {
            os << indent
               << "template<>"
               << fhg::util::deeper (indent)
               << "inline value_type to_value<"
               << fhg::util::join (tname.begin(), tname.end(), "::")
               << "> (const "
               << fhg::util::join (tname.begin(), tname.end(), "::")
               << "& x)"
               << fhg::util::cpp::block::open (indent)
               << indent << "return "
               << fhg::util::join (tname.begin(), std::prev (tname.end()), "::")
               << "::to_value (x);"
               << fhg::util::cpp::block::close (indent);

            os << indent
               << "template<>"
               << fhg::util::deeper (indent)
               << "inline "
               << fhg::util::join (tname.begin(), tname.end(), "::")
               << " from_value<"
               << fhg::util::join (tname.begin(), tname.end(), "::")
               << "> (value_type const& v)"
               << fhg::util::cpp::block::open (indent)
               << indent << "return "
               << fhg::util::join (tname.begin(), std::prev (tname.end()), "::")
               << "::from_value (v);"
               << fhg::util::cpp::block::close (indent);
          }

          os << fhg::util::cpp::ns::close (indent);
          os << fhg::util::cpp::ns::close (indent);
          os << fhg::util::cpp::ns::close (indent);

          return os;
        }
        std::ostream& operator<< (std::ostream& os, const header_op& h)
        {
          return h (os);
        }

        namespace
        {
          class print_header_op_signature
            : public boost::static_visitor<std::ostream&>
          {
          public:
            print_header_op_signature (std::ostream& os)
              : _os (os)
            {}
            std::ostream& operator() (const std::string& s) const
            {
              return _os << "//" << s;
            }
            std::ostream& operator() (const structured_type& s) const
            {
              return _os << header_op (s);
            }
          private:
            std::ostream& _os;
          };
        }

        header_op_signature::header_op_signature (const signature_type& signature)
          : _signature (signature)
        {}
        std::ostream& header_op_signature::operator() (std::ostream& os) const
        {
          return boost::apply_visitor (print_header_op_signature (os), _signature);
        }
        std::ostream& operator<< (std::ostream& os, const header_op_signature& h)
        {
          return h (os);
        }

        namespace
        {
          class print_from_value : public printer
          {
          public:
            print_from_value ( std::ostream& os
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
              _os << fhg::util::deeper (_indent) << (_first ? '(' : ',') << ' ';

              if (!is_literal (f.second))
              {
                _os << f.second << "::from_value (pnet::field";
              }
              else
              {
                _os << "pnet::field_as< " << complete (f.second) << " >";
              }

              _os << " ("
                  << "\"" << f.first << "\""
                  << ", v"
                  << ", std::string(\"" << f.second << "\")"
                  << ")";

              if (!is_literal (f.second))
              {
                _os << ")";
              }

              _first = false;
            }
            void _field_struct (const std::pair<std::string, structure_type>& s) const
            {
              _os << fhg::util::deeper (_indent)
                  << (_first ? '(' : ',') << ' '
                  << s.first << "::from_value (pnet::field"
                  << " ("
                  << "\"" << s.first << "\""
                  << ", v"
                  << ", pnet::signature_of ("
                  << s.first << "::to_value (" << s.first << "::" << s.first << "())"
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
                os << type << "::to_value (" << "x." << name << ")";
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

              from_value (s);
              to_value (s);
              show (s.first);

              _os << ns::close (_indent);
            }
            void _field (const std::pair<std::string, std::string>&) const
            {}
            void _field_struct (const std::pair<std::string, structure_type>& s) const
            {
              traverse (*this, s);
            }
          private:
            void from_value
              (const std::pair<std::string, structure_type>& s) const
            {
              namespace block = fhg::util::cpp::block;

              _os << _indent
                  << s.first << " from_value "
                  << "(const pnet::type::value::value_type& v)"
                  << block::open (_indent)
                  << _indent << "return " << s.first;

              if (s.second.empty())
              {
                _os << "();";
              }
              else
              {
                bool first (true);

                traverse (print_from_value (_os, _indent, first), s);

                _os << fhg::util::deeper (_indent) << ");";
              }

              _os << block::close (_indent);
            }
            void to_value
              (const std::pair<std::string, structure_type>& s) const
            {
              namespace block = fhg::util::cpp::block;

              _os << _indent
                  << "pnet::type::value::value_type to_value (const " << s.first << "& x)"
                  << block::open (_indent)
                  << _indent << "pnet::type::value::value_type v;";

              traverse (print_impl_poke (_os, _indent), s);

              _os << _indent << "return v;"
                  << block::close (_indent);
            }
            void show (const std::string& name) const
            {
              namespace block = fhg::util::cpp::block;

              _os << _indent
                  << "std::ostream& operator<< (std::ostream& os, const " << name << "& x)"
                  << block::open (_indent)
                  << _indent
                  << "return os << pnet::type::value::show (to_value (x));"
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

          os << fhg::util::cpp::include ("we/field.hpp");
          os << fhg::util::cpp::include ("we/signature_of.hpp");
          os << fhg::util::cpp::include ("we/type/value/poke.hpp");
          os << fhg::util::cpp::include ("we/type/value/show.hpp");
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
