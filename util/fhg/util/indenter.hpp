// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_INDENTER_HPP
#define FHG_UTIL_INDENTER_HPP

#include <fhg/util/ostream_modifier.hpp>

#include <iosfwd>

namespace fhg
{
  namespace util
  {
    class indenter : public ostream::modifier
    {
    public:
      indenter (const unsigned int = 0);
      indenter& operator++();
      indenter operator++ (int);
      indenter& operator--();
      indenter operator-- (int);
      std::ostream& operator() (std::ostream&) const;

    private:
      unsigned int _depth;
    };

    class deeper : public ostream::modifier
    {
    public:
      deeper (indenter&);
      ~deeper();
      std::ostream& operator() (std::ostream&) const;
    private:
      indenter& _indenter;
    };
  }
}

#endif
