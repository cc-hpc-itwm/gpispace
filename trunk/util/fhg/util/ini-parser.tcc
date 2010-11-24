// -*- mode: c++; -*-

#include "ini-parser.hpp"
#include "ini-parser-except.hpp"

#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace fhg
{
  namespace util
  {
    namespace ini
    {
      namespace detail
      {
        // predicates
        bool is_comment (std::string const & line)
        {
          if (line.size())
          {
            if (line[0] == ';') return true;
            if (line[0] == '#') return true;
          }
          return false;
        }

        bool is_ignored (std::string const & line)
        {
          if (line.empty()) return true;
          if (is_comment (line)) return true;
          return false;
        }

        bool is_section_header (std::string const & line)
        {
          if (line.size())
          {
            if (line[0] == '[' && line[line.size()-1] == ']')
              return true;
          }
          return false;
        }

        bool is_include (std::string const & line)
        {
          if (line.find ("%include ") == 0)
          {
            return true;
          }
          return false;
        }

        bool is_key_value (std::string const & line)
        {
          if (line.find ("=") != std::string::npos)
          {
            return true;
          }
          return false;
        }

        struct state_t
        {
          state_t (entry_handler_t ahandler)
            : base ("")
            , file ("")
            , dir ("")
            , section ("")
            , line ("")
            , lineno (0)
            , handler (ahandler)
            , Werror (false)
          {}

          template <typename T>
          bool is_enabled (T const &)
          {
            return true;
          }

          template <typename T>
          void warn  (T const & t)
          {
            if (is_enabled(t))
            {
              if (Werror)
              {
                // warnings treated as errors
                error (t);
              }
            }
          }

          void error (std::exception const & e)
          {
            std::cerr << "E: parser error in line " << lineno << ": " << line << ": " << e.what() << std::endl;
            throw e;
          }

          std::string base;
          std::string file;
          std::string dir;
          std::string section;
          std::string line;
          int lineno;
          entry_handler_t handler;

          bool Werror;
        };

        void parse_section_header (std::string const & line, state_t & state)
        {
          state.section = line.substr (1, line.size() - 2);
          boost::trim (state.section);
        }

        void parse_include (std::string const & line, state_t & state)
        {
          state.error (exception::parse_error("%include is not yet implemented"));
        }

        void parse_key_value (std::string const & line, state_t & state)
        {
          std::string::size_type split_pos (line.find ("="));
          std::string key (line.substr(0, split_pos));
          std::string val (line.substr(split_pos+1));

          boost::trim (key);
          boost::trim (val);

          int ec = state.handler (state.section, key, val);
          if (ec)
          {
            state.error (exception::parse_error("handler failed: " + boost::lexical_cast<std::string>(ec)));
          }
        }

        void parse_line (std::string & line, state_t & state)
        {
          boost::trim (line);

          if (is_ignored (line))
          {
            return;
          }
          else if (is_section_header (line))
          {
            parse_section_header (line, state);
          }
          else if (is_include (line))
          {
            parse_include (line, state);
          }
          else if (is_key_value (line))
          {
            parse_key_value (line, state);
          }
          else
          {
            state.error (exception::parse_error("invalid line: " + line));
          }
        }

        void parse_impl (std::istream & is, state_t & state)
        {
          while (is)
          {
            std::getline (is, state.line);
            if (! is)
              break;
            ++state.lineno;
            parse_line (state.line, state);
          }
        }
      }

      void parse (std::istream & is, entry_handler_t handler)
      {
        detail::state_t state (handler);
        detail::parse_impl (is, state);
      }

      void parse (std::string const & path, entry_handler_t handler)
      {
        detail::state_t state (handler);
        state.base = path;

        std::ifstream ifs (path.c_str());
        if (! ifs)
        {
          throw exception::parse_error ("could not open file: "+path);
        }
        else
        {
          detail::parse_impl (ifs, state);
        }
      }
    }
  }
}
