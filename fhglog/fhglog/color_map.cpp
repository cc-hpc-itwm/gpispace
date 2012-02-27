#include "color_map.hpp"

#include <sstream>

namespace fhg
{
  namespace log
  {
    color_map_t::color_map_t ()
    {
      // flags:
      //    0 - normal
      //    5 - blink
      m_color_table[LogLevel::TRACE] = color_escape_code(FG_WHITE, BG_DEFAULT, 0);
      m_color_table[LogLevel::DEBUG] = color_escape_code(FG_WHITE, BG_DEFAULT, 0);
      m_color_table[LogLevel::INFO]  = color_escape_code(FG_CYAN);
      m_color_table[LogLevel::WARN]  = color_escape_code(FG_YELLOW);
      m_color_table[LogLevel::ERROR] = color_escape_code(FG_RED);
      m_color_table[LogLevel::FATAL] = color_escape_code(FG_MAGENTA, BG_DEFAULT, 0);
    }

    std::string const & color_map_t::reset_escape_code ()
    {
      static std::string s ("\033[0m");
      return s;
    }

    std::string color_map_t::color_escape_code ( color_map_t::ColorEnum fg
                                               , color_map_t::ColorEnum bg
                                               , int flags
                                               )
    {
      std::ostringstream os;
      os << "\033[" << flags << ";" << fg << ";" << bg << "m";
      return os.str();
    }

    std::string const & color_map_t::colorize (const LogLevel &lvl) const
    {
      return m_color_table.at (lvl);
    }

    std::string const & color_map_t::operator[](const LogLevel &level) const
    {
      return const_cast<color_map_t&>(*this).operator[](level);
    }
    std::string & color_map_t::operator[](const LogLevel &level)
    {
      if (m_color_table.find(level) == m_color_table.end())
      {
        m_color_table[level] = reset_escape_code();
      }
      return m_color_table[level];
    }
  }
}
