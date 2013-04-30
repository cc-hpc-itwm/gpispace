// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/cpp.hpp>
#include <we/type/signature/show.hpp>

#include <we/type/value/name.hpp>

#include <fhg/util/indenter.hpp>
#include <fhg/util/cpp/block.hpp>
#include <fhg/util/cpp/namespace.hpp>
#include <fhg/util/cpp/struct.hpp>

#include <boost/foreach.hpp>

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
          namespace block = fhg::util::cpp::block;
          namespace ns = fhg::util::cpp::ns;
          namespace structure = fhg::util::cpp::structure;

          std::ostream& decl ( std::ostream& os
                             , fhg::util::indenter& indent
                             , const std::string& name
                             , const std::string& type
                             )
          {
            return os << fhg::util::deeper (indent)
                      << value::typename_of (type) << " " << name <<  ";";
          }

          class header_struct : public boost::static_visitor<std::ostream&>
          {
          public:
            header_struct (std::ostream&, fhg::util::indenter&);
            std::ostream& operator() (const std::pair< std::string
                                                     , structure_type
                                                     >&
                                     ) const;
          private:
            std::ostream& _os;
            fhg::util::indenter& _indenter;
          };

          class header_field_struct
            : public boost::static_visitor<std::ostream&>
          {
          public:
            header_field_struct ( std::ostream& os
                                , fhg::util::indenter& indenter
                                , std::ostream& field
                                )
              : _os (os)
              , _indenter (indenter)
              , _field (field)
            {}

            std::ostream& operator() (const std::pair< std::string
                                                     , structure_type
                                                     >& s
                                     ) const
            {
              decl (_field, _indenter, s.first, s.first);

              return header_struct (_os, _indenter)(s);
            }
          private:
            std::ostream& _os;
            fhg::util::indenter& _indenter;
            std::ostream& _field;
          };

          class header_field : public boost::static_visitor<std::ostream&>
          {
          public:
            header_field ( std::ostream& os
                         , fhg::util::indenter& indenter
                         , std::ostream& field
                         )
              : _os (os)
              , _indenter (indenter)
              , _field (field)
            {}

            std::ostream& operator() (const std::pair< std::string
                                                     , std::string
                                                     >& f
                                     ) const
            {
              return decl (_field, _indenter, f.first, f.second);
            }
            std::ostream& operator() (const structured_type& s) const
            {
              return boost::apply_visitor
                (header_field_struct (_os, _indenter, _field), s);
            }
          private:
            std::ostream& _os;
            fhg::util::indenter& _indenter;
            std::ostream& _field;
          };

          header_struct::header_struct ( std::ostream& os
                                       , fhg::util::indenter& indenter
                                       )
            : _os (os)
            , _indenter (indenter)
          {}
          std::ostream&
          header_struct::operator() (const std::pair< std::string
                                                    , structure_type
                                                    >& s
                                    ) const
          {
            _os << ns::open (_indenter, s.first);

            std::ostringstream field;
            BOOST_FOREACH (const field_type& f, s.second)
            {
              boost::apply_visitor (header_field (_os, _indenter, field), f);
            }
            return _os
              << structure::open (_indenter, "type")
              << field.str()
              << _indenter << "type();"
              << _indenter << "type (const pnet::type::value::value_type&);"
              << structure::close (_indenter)
              << ns::close (_indenter);
          }
        }

        header::header (const structured_type& structured)
          : _structured (structured)
        {}
        std::ostream& header::operator() (std::ostream& os) const
        {
          fhg::util::indenter indenter;

          return boost::apply_visitor ( header_struct (os, indenter)
                                      , _structured
                                      );
        }
        std::ostream& operator<< (std::ostream& os, const header& h)
        {
          return h (os);
        }
      }
    }
  }
}
