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
      //! \todo build parse::positon from istream
      class position
      {
      public:
        position (const std::string&);
        position ( const std::string::const_iterator& begin
                 , const std::string::const_iterator& end
                 );

        const char& operator*() const
        {
          return *_pos;
        }
        void operator++()
        {
          ++_k;
          ++_pos;
        }
        void advance (std::size_t d)
        {
          _k += d;
          _pos += d;
        }
        bool end() const
        {
          return _pos == _end;
        }
        virtual std::size_t eaten() const
        {
          return _k;
        }

        std::string consumed() const;
        std::string rest() const;

      private:
        std::size_t _k;
        std::string::const_iterator _pos;
        const std::string::const_iterator _begin;
        const std::string::const_iterator _end;
      };
    }
  }
}

#endif
