#include <ctype.h>
#include <iostream>

#include "parser.hpp"

#include <gspc/net/frame.hpp>
#include <gspc/net/limits.hpp>

#include <boost/lexical_cast.hpp>

namespace gspc
{
  namespace net
  {
    namespace parse
    {
      parser::parser ()
        : m_frame_state (frame_start)
        , m_error (E_BAD_REQUEST)
        , m_buffer ()
        , m_header_key ()
        , m_remaining_body_bytes (0)
      {}

      void parser::reset ()
      {
        m_frame_state = frame_start;
        m_error = E_BAD_REQUEST;
        m_buffer.clear ();
        m_header_key.clear ();
        m_remaining_body_bytes = 0;
      }

      namespace
      {
        bool is_carriage_return (const char c)
        {
          return c == 13;
        }
        bool is_line_feed (const char c)
        {
          return c == 10;
        }
        bool is_header_separator (const char c)
        {
          return c == ':';
        }
        bool is_null (const char c)
        {
          return c == 0;
        }
      }

      result_t parser::parse ( const char *begin
                             , const char *end
                             , gspc::net::frame & frame
                             )
      {
        size_t consumed = 0;

        while (begin != end)
        {
          state_t state = consume (frame, *begin++);
          ++consumed;

          if (state == PARSE_FINISHED)
            return result_t (consumed, state, E_OK);
          else if (state == PARSE_FAILED)
            return result_t (consumed, state, m_error);
        }

        return result_t ( consumed
                        , PARSE_NEED_MORE_DATA
                        , m_error
                        );
      }

      inline state_t parser::consume ( gspc::net::frame & frame
                                     , const char c
                                     )
      {
        using namespace gspc::net::limits;

        switch (m_frame_state)
        {
        case frame_start:
          {
            if (is_line_feed (c))
              return PARSE_FINISHED;
            if (is_carriage_return (c))
            {
              m_frame_state = frame_start_line_feed;
            }
            else if (isprint (c))
            {
              m_buffer.push_back (c);
              m_frame_state = command;
            }
            else
            {
              return PARSE_FAILED;
            }

            return PARSE_NEED_MORE_DATA;
          }
        case frame_start_line_feed:
          {
            if (is_line_feed (c))
              return PARSE_FINISHED;
            else
            {
              return PARSE_FAILED;
            }
          }
        case command:
          {
            if (is_line_feed (c))
            {
              frame.set_command (m_buffer);
              m_buffer.clear ();
              m_frame_state = header_start;
              return PARSE_NEED_MORE_DATA;
            }

            if (is_carriage_return (c))
            {
              m_frame_state = command_line_feed;
            }
            else if (isprint (c))
            {
              if (m_buffer.size () < max_command_length ())
              {
                m_buffer.push_back (c);
              }
              else
              {
                m_error = E_COMMAND_TOO_LONG;
                return PARSE_FAILED;
              }
            }
            else
            {
              return PARSE_FAILED;
            }

            return PARSE_NEED_MORE_DATA;
          }
        case command_line_feed:
          {
            if (is_line_feed (c))
            {
              frame.set_command (m_buffer);
              m_buffer.clear ();
              m_frame_state = header_start;
              return PARSE_NEED_MORE_DATA;
            }
            else
            {
              return PARSE_FAILED;
            }
          }
        case header_start:
          {
            if (is_line_feed (c))
            {
              if (frame.has_header ("content-length"))
              {
                try
                {
                  m_remaining_body_bytes = boost::lexical_cast<std::size_t>
                    (*frame.get_header ("content-length"));
                }
                catch (boost::bad_lexical_cast const &)
                {
                  return PARSE_FAILED;
                }

                if (m_remaining_body_bytes)
                  m_frame_state = body_with_content_length;
                else
                  m_frame_state = body_with_content_length_final_null;
              }
              else
              {
                m_frame_state = body_without_content_length;
              }
              return PARSE_NEED_MORE_DATA;
            }

            if (is_header_separator (c))
            {
              return PARSE_FAILED;
            }

            if (is_carriage_return (c))
            {
              m_frame_state = header_start_line_feed;
            }
            else if (isprint (c))
            {
              m_buffer.push_back (c);
              m_frame_state = header_name;
            }
            else
            {
              return PARSE_FAILED;
            }

            return PARSE_NEED_MORE_DATA;
          }
        case header_start_line_feed:
          {
            if (is_line_feed (c))
            {
              if (frame.has_header ("content-length"))
              {
                try
                {
                  m_remaining_body_bytes = boost::lexical_cast<std::size_t>
                    (frame.get_header ("content-length"));
                }
                catch (boost::bad_lexical_cast const &)
                {
                  return PARSE_FAILED;
                }

                m_frame_state = body_with_content_length;
              }
              else
              {
                m_frame_state = body_without_content_length;
              }
              return PARSE_NEED_MORE_DATA;
            }
            else
            {
              return PARSE_FAILED;
            }
          }
        case header_name:
          {
            if (is_line_feed (c) || is_carriage_return (c))
            {
              return PARSE_FAILED;
            }

            if (is_header_separator (c))
            {
              m_frame_state = header_value;
              m_header_key = m_buffer;
              m_buffer.clear ();
            }
            else if (isprint (c))
            {
              if (m_buffer.size () < max_header_key_length ())
              {
                m_buffer.push_back (c);
              }
              else
              {
                m_error = E_HEADER_FIELDS_TOO_LARGE;
                return PARSE_FAILED;
              }
            }
            else
            {
              return PARSE_FAILED;
            }

            return PARSE_NEED_MORE_DATA;
          }
        case header_value:
          {
            if (is_line_feed (c))
            {
              frame.set_header (m_header_key, m_buffer);
              m_header_key.clear ();
              m_buffer.clear ();
              m_frame_state = header_start;
              return PARSE_NEED_MORE_DATA;
            }

            if (is_carriage_return (c))
            {
              m_frame_state = header_value_line_feed;
            }
            else if (isprint (c))
            {
              if (m_buffer.size () < max_header_value_length ())
              {
                m_buffer.push_back (c);
              }
              else
              {
                m_error = E_HEADER_FIELDS_TOO_LARGE;
                return PARSE_FAILED;
              }
            }
            else
            {
              return PARSE_FAILED;
            }

            return PARSE_NEED_MORE_DATA;
          }
        case header_value_line_feed:
          {
            if (is_line_feed (c))
            {
              frame.set_header (m_header_key, m_buffer);
              m_header_key.clear ();
              m_buffer.clear ();
              m_frame_state = header_start;
              return PARSE_NEED_MORE_DATA;
            }
            else
            {
              return PARSE_FAILED;
            }
          }
        case body_without_content_length:
          {
            if (is_null (c))
            {
              return PARSE_FINISHED;
            }
            else
            {
              frame.add_body (&c, 1);
              return PARSE_NEED_MORE_DATA;
            }
          }
        case body_with_content_length:
          {
            if (m_remaining_body_bytes)
            {
              frame.add_body (&c, 1);
              --m_remaining_body_bytes;
            }

            if (m_remaining_body_bytes)
            {
              return PARSE_NEED_MORE_DATA;
            }
            else
            {
              m_frame_state = body_with_content_length_final_null;
              return PARSE_NEED_MORE_DATA;
            }
          }
        case body_with_content_length_final_null:
          {
            if (is_null (c))
            {
              return PARSE_FINISHED;
            }
            else
            {
              return PARSE_FAILED;
            }
          }
        default:
          abort ();
        }

        return gspc::net::parse::PARSE_FAILED;
      }
    }
  }
}
