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
            , section_id ("")
            , section_id_set (false)
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

          template <typename Ex>
          void warn  (Ex const & e)
          {
            if (is_enabled(e))
            {
              if (Werror)
              {
                std::cerr << "W: Warnings treated as errors!" << std::endl;
                // warnings treated as errors
                error (e);
              }
              else
              {
                std::cerr << "W: parser error in line " << lineno << ": " << line << ":" << std::endl;
                std::cerr << "     " << e.what() << std::endl;
              }
            }
          }

          template <typename Ex>
          void error (Ex const & e)
          {
            std::cerr << "E: parser error in line " << lineno << ": " << line << ":" << std::endl;
            std::cerr << "     " << e.what() << std::endl;
            throw e;
          }

          std::string base;
          std::string file;
          std::string dir;
          std::string section;
          std::string section_id;
          bool section_id_set;
          std::string line;
          int lineno;
          entry_handler_t handler;

          bool Werror;
        };

        void split_section_head ( std::string const & hd
                                , state_t & state
                                )
        {
          std::string::size_type sec_id_pos (hd.find (" "));
          if (sec_id_pos != std::string::npos)
          {
            std::string sec_name = hd.substr(0, sec_id_pos);
            std::string sec_id = hd.substr(sec_id_pos+1);
            boost::trim (sec_name);
            boost::trim (sec_id);
            if ( sec_id.size() < 2
               || (sec_id[0] != '"' && sec_id[sec_id.size()-1] != '"')
               )
            {
              state.error (exception::parse_error("invalid section header"));
            }
            else
            {
              sec_id = sec_id.substr(1, sec_id.size() - 2);
              state.section    = sec_name;
              state.section_id = sec_id;
              state.section_id_set = true;
            }
          }
          else
          {
            state.section = hd;
            state.section_id_set = false;
          }
        }

        void parse_section_header (std::string const & line, state_t & state)
        {
          std::string section_head(line.substr (1, line.size() - 2));
          boost::trim (section_head);
          split_section_head ( section_head, state );
        }

        void parse_include (std::string const & line, state_t & state)
        {
          state.warn
            (exception::parse_error
            ("%include is not yet implemented")
            );
        }

        void parse_key_value (std::string const & line, state_t & state)
        {
          std::string::size_type split_pos (line.find ("="));
          std::string key (line.substr(0, split_pos));
          std::string val (line.substr(split_pos+1));

          boost::trim (key);
          boost::trim (val);

          if (val.size())
          {
            if (val[0] == '"')
            {
              if (val[val.size()-1] != '"')
              {
                state.error
                  (exception::parse_error
                  ("invalid value: " + val + ": missing closing \"")
                  );
              }
              else
              {
                val = val.substr(1, val.size() - 2);
              }
            }
          }

          if (state.section.empty())
          {
            state.error
              (exception::parse_error
              ("key does not contain a section: " + key)
              );
          }

          int ec (0);
          if (! state.section_id_set)
          {
            ec = state.handler ( key_desc_t ( state.section
                                            , key
                                            )
                               , val
                               );
          }
          else
          {
            ec = state.handler ( key_desc_t ( state.section
                                            , key
                                            , state.section_id
                                            )
                               , val
                               );
          }

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
        state.file = "<STDIN>";
        detail::parse_impl (is, state);
      }

      void parse ( std::istream & is
                 , std::string const & stream_desc
                 , entry_handler_t handler
                 )
      {
        detail::state_t state (handler);
        state.file = stream_desc;
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
