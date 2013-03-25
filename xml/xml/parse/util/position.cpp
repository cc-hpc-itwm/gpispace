// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/position.hpp>

#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      namespace
      {
        void inc_line_and_column ( char* pos
                                 , const char* end
                                 , unsigned int& line
                                 , unsigned int& column
                                 )
        {
          for (; pos != end; ++pos)
          {
            ++column;

            if (*pos == '\n')
            {
              column = 0;
              ++line;
            }
          }
        }
      }

      position_type::position_type ( const char* begin
                                   , const char* pos
                                   , const boost::filesystem::path& path
                                   )
        : _line (1)
        , _column (0)
        , _path (path)
      {
        inc_line_and_column (const_cast<char*>(begin), pos, _line, _column);
      }

      const unsigned int& position_type::line() const
      {
        return _line;
      }
      const unsigned int& position_type::column() const
      {
        return _column;
      }
      const boost::filesystem::path& position_type::path() const
      {
        return _path;
      }
      const char* const& position_type::pos() const
      {
        return _pos;
      }

      std::ostream& operator<< (std::ostream& os, const position_type& p)
      {
        return os << "[" << p.path().string()
                  << ":" << p.line()
                  << ":" << p.column()
                  << "]";
      }
    }
  }
}
