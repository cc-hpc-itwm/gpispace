// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_PARSE_POSITION_HPP
#define _FHG_UTIL_PARSE_POSITION_HPP

#include <string>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      class position
      {
      private:
        std::size_t & _k;
        std::string::const_iterator & _pos;
        const std::string::const_iterator & _end;
      public:
        position ( std::size_t & k
                 , std::string::const_iterator & pos
                 , const std::string::const_iterator & end
                 )
          : _k (k), _pos (pos), _end (end)
        {}

        std::string rest (void) { return std::string (_pos, _end); }
        const char & operator * (void) const { return *_pos; }
        void operator ++ (void) { ++_k; ++_pos; }
        bool end (void) const { return _pos == _end; }
        const std::size_t & operator () (void) const { return _k; }

        void skip_spaces()
        {
          while (_pos != _end && isspace (*_pos))
          {
            ++_pos;
          }
        }
      };
    }
  }
}

#endif
