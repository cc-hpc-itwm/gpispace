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
      public:
        position ( std::size_t& k
                 , std::string::const_iterator& pos
                 , const std::string::const_iterator& end
                 );
        std::string rest() const;
        const char& operator*() const;
        void operator++();
        bool end() const;
        const std::size_t& operator() () const;
        void skip_spaces();

      private:
        std::size_t& _k;
        std::string::const_iterator& _pos;
        const std::string::const_iterator& _end;
      };
    }
  }
}

#endif
