// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_PARSE_POSITION_HPP
#define _FHG_UTIL_PARSE_POSITION_HPP

#include <string>

#include <boost/function.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
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

        std::string consumed() const;
        std::string rest() const;
        const std::size_t& operator() () const;

        void skip_spaces();
        void require (const std::string&);
        void require (const char&);
        std::string identifier();
        char character();
        std::string until (const char c, const char escape = '\\');
        void list ( const char open, const char sep, const char close
                  , const boost::function<void (position&)>&
                  , const bool skip_space_before_element = true
                  , const bool skip_space_after_element = true
                  );

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
