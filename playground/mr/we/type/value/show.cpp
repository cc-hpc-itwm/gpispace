// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/show.hpp>
#include <we/type/value/util/show.hpp>

namespace
{
  using pnet::type::value::value_type;
  using pnet::type::value::structured_type;
  using pnet::type::value::show::parens_type;

  class parens_of : public boost::static_visitor<parens_type>
  {
  public:
    parens_type operator() (const std::list<value_type>&) const
    {
      return parens_type ("(", ", ", ")");
    }
    parens_type operator() (const std::map<value_type, value_type>&) const
    {
      return parens_type ("[", ", ", "]");
    }
    parens_type operator() (const std::vector<value_type>&) const
    {
      return parens_type ("(", ", ", ")");
    }
    parens_type operator() (const std::set<value_type>&) const
    {
      return parens_type ("{", ", ", "}");
    }
    parens_type operator() (const structured_type&) const
    {
      return parens_type ("[", ", ", "]");
    }
  };

  class show_literal : public boost::static_visitor<std::ostream&>
  {
  public:
    std::ostream& operator() (std::ostream& os, const control& c) const
    {
      return os << c;
    }
    std::ostream& operator() (std::ostream& os, const bool& b) const
    {
      return os << b;
    }
    std::ostream& operator() (std::ostream& os, const int& i) const
    {
      return os << i;
    }
    std::ostream& operator() (std::ostream& os, const long& i) const
    {
      return os << i << "L";
    }
    std::ostream& operator() (std::ostream& os, const unsigned int& i) const
    {
      return os << i << "U";
    }
    std::ostream& operator() (std::ostream& os, const unsigned long& i) const
    {
      return os << i << "UL";
    }
    std::ostream& operator() (std::ostream& os, const float& f) const
    {
      return os << f << "f";
    }
    std::ostream& operator() (std::ostream& os, const double& d) const
    {
      return os << d;
    }
    std::ostream& operator() (std::ostream& os, const char& c) const
    {
      return os << "'" << c << "'";
    }
    std::ostream& operator() (std::ostream& os, const std::string& s) const
    {
      return os << "\"" << s << "\"";
    }
    std::ostream& operator() (std::ostream& os, const bitsetofint::type& bs) const
    {
      return os << bs;
    }
    std::ostream& operator() (std::ostream& os, const bytearray::type& ba) const
    {
      return os << ba;
    }
  };
}

std::ostream& operator<< ( std::ostream& os
                         , const pnet::type::value::value_type& v
                         )
{
  const std::ios_base::fmtflags ff (os.flags());
  os << std::showpoint << std::boolalpha;
  parens_of parens_of;
  show_literal show_literal;
  boost::apply_visitor
    ( pnet::type::value::show::visitor_show
      ( os
      , boost::apply_visitor (show_literal)
      , boost::apply_visitor (parens_of)
      , " := "
      )
    , v
    );
  os.flags (ff);
  return os;
}
