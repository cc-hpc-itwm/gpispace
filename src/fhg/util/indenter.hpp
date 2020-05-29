#pragma once

#include <util-generic/ostream/modifier.hpp>

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
      virtual std::ostream& operator() (std::ostream&) const override;

    private:
      unsigned int _depth;
    };

    class deeper : public ostream::modifier
    {
    public:
      deeper (indenter&);
      virtual std::ostream& operator() (std::ostream&) const override;
    private:
      indenter& _indenter;
    };
  }
}
