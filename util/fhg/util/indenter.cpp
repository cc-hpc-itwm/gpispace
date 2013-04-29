// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/indenter.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    indenter::indenter (const unsigned int depth)
      : _depth (depth)
    {}
    indenter& indenter::operator++()
    {
      ++_depth;
      return *this;
    }
    indenter indenter::operator++ (int)
    {
      indenter old (*this);
      ++_depth;
      return old;
    }
    indenter& indenter::operator--()
    {
      --_depth;
      return *this;
    }
    indenter indenter::operator-- (int)
    {
      indenter old (*this);
      --_depth;
      return old;
    }
    std::ostream& indenter::operator() (std::ostream& os) const
    {
      for (unsigned int i (0); i < _depth; ++i)
      {
        os << "  ";
      }
      return os;
    }

    std::ostream& operator<< (std::ostream& os, const indenter& indent)
    {
      return indent (os);
    }
  }
}
