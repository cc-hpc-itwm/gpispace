// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_UTIL_HPP
#define FUSE_UTIL_HPP 1

#include <string>

#include <boost/optional.hpp>
#include <cctype>

#include <sstream>

#define STRCONST(C,S)                  \
  static inline std::string C ()       \
  {                                    \
    static const std::string s (S);    \
                                       \
    return s;                          \
  }
#define CONST(C) STRCONST(C,#C)

namespace gpifs
{
  namespace util
  {
    namespace parse
    {
      template<typename IT>
      class parser
      {
      public:
        typedef boost::optional<std::string> error_t;

        parser (IT & pos, const IT & end)
          : _begin (pos)
          , _end (end)
          , _pos (pos)
          , _error ()
        {}

        bool end () const { return _pos == _end; }
        void operator ++ () { ++_pos; }
        const char & operator * () const { return *_pos; }

        std::string input () const { return std::string (_begin, _end); }
        std::string rest () const { return std::string (_pos, _end); }
        std::size_t consumed () const { return std::distance (_begin, _pos); }

        const error_t & error () const { return _error; }
        void error_set (const std::string & err) { _error = err; }

        std::string error_string (const std::string & msg) const
        {
          std::ostringstream err;

          err << msg << ":" << std::endl
              << "input was:" << std::endl
              << input() << std::endl
            ;

          for (std::size_t i (0); i < consumed(); ++i)
            {
              err << " ";
            }

          err << "^" << std::endl;

          if (error())
            {
              err << *error() << std::endl;
            }

          return err.str();
        }

      private:
        const IT _begin;
        const IT & _end;
        IT & _pos;
        error_t _error;
      };

      typedef parser<std::string::const_iterator> parser_string;

      // ******************************************************************* //

      template<typename IT>
      static inline bool
      require ( std::string::const_iterator what_pos
              , const std::string::const_iterator & what_end
              , parser<IT> & parser
              )
      {
        while (what_pos != what_end)
          {
            if (parser.end() || tolower (*parser) != tolower (*what_pos))
              {
                parser.error_set
                  ("expected: '" + std::string (what_pos, what_end) + "'");

                return false;
              }

            ++what_pos;
            ++parser;
          }

        return true;
      }

      template<typename IT>
      static inline bool
      require (const std::string & what, parser<IT> & parser)
      {
        return require (what.begin(), what.end(), parser);
      }

      // ******************************************************************* //

      template<typename IT>
      static inline void skip_space (parser<IT> & parser)
      {
        while (!parser.end() && isspace (*parser))
          {
            ++parser;
          }
      }
    } // namespace parse
  } // namespace util
} // namespace gpifs

#endif
