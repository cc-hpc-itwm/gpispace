// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_EXPR_PARSE_POSITION_HPP
#define _WE_EXPR_PARSE_POSITION_HPP

#include <string>

namespace expr
{
  namespace parse
  {
    class position
    {
    private:
      unsigned int & _k;
      std::string::const_iterator & _pos;
      const std::string::const_iterator & _end;
    public:
      position ( unsigned int & k
               , std::string::const_iterator & pos
               , const std::string::const_iterator & end
               )
        : _k (k), _pos (pos), _end (end)
      {}

      char operator * (void) const { return *_pos; }
      void operator ++ (void) { ++_k; ++_pos; }
      bool end (void) const { return _pos == _end; }
      const unsigned int & operator () (void) const { return _k; }
    };
  }
}

#endif
