// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/num/show.hpp>

#include <iostream>

namespace
{
  class visitor_show : public boost::static_visitor<std::ostream&>
  {
  public:
    visitor_show (std::ostream& os)
      : _os (os)
    {}

    std::ostream& operator() (const int& x) const
    {
      return _os << x;
    }
    std::ostream& operator() (const long& x) const
    {
      return _os << x << "L";
    }
    std::ostream& operator() (const unsigned int& x) const
    {
      return _os << x << "U";
    }
    std::ostream& operator() (const unsigned long& x) const
    {
      return _os << x << "UL";
    }
    std::ostream& operator() (const float& x) const
    {
      return _os << x << "f";
    }
    std::ostream& operator() (const double& x) const
    {
      return _os << x;
    }

  private:
    std::ostream& _os;
  };
}

std::ostream& operator<< (std::ostream& os, const fhg::util::num_type& v)
{
  const std::ios_base::fmtflags ff (os.flags());
  os << std::showpoint;
  boost::apply_visitor (visitor_show (os), v);
  os.flags (ff);
  return os;
}
