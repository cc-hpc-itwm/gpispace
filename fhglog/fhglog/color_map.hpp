#ifndef FHGLOG_COLOR_MAP_HPP
#define FHGLOG_COLOR_MAP_HPP 1

#include <map>

#include <fhglog/LogLevel.hpp>

namespace fhg
{
  namespace log
  {
    class color_map_t
    {
    public:
      enum ColorEnum
        {
          FG_BLACK      = 30
          , FG_RED      = 31
          , FG_GREEN    = 32
          , FG_YELLOW   = 33
          , FG_BLUE     = 34
          , FG_MAGENTA  = 35
          , FG_CYAN     = 36
          , FG_WHITE    = 37
          , FG_DEFAULT  = 39

          , BG_BLACK    = 40
          , BG_RED      = 41
          , BG_GREEN    = 42
          , BG_YELLOW   = 43
          , BG_BLUE     = 44
          , BG_MAGENTA  = 45
          , BG_CYAN     = 46
          , BG_WHITE    = 47
          , BG_DEFAULT  = 49
        };

      color_map_t ();

      std::string const & colorize (const LogLevel & lvl) const;
      std::string const & operator[](const LogLevel & lvl) const;
      std::string & operator[](const LogLevel & lvl);

      static std::string const & reset_escape_code ();
      static std::string color_escape_code ( ColorEnum fg
                                           , ColorEnum bg = BG_DEFAULT
                                           , int flags = 0
                                           );
    private:
      typedef std::map<LogLevel::Level, std::string> color_table_t;
      color_table_t m_color_table;
    };
  }
}

#endif
