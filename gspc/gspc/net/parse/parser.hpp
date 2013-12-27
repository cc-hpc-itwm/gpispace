#ifndef GSPC_NET_PARSER_HPP
#define GSPC_NET_PARSER_HPP

#include <string>

#include <gspc/net/error.hpp>
#include <gspc/net/frame_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace parse
    {
      enum state_t
        {
          PARSE_FAILED
        , PARSE_NEED_MORE_DATA
        , PARSE_FINISHED
        };

      struct result_t
      {
        result_t ()
          : consumed (0)
          , state (PARSE_FAILED)
          , reason (E_OK)
        {}

        result_t (size_t consumed, state_t s, error_code_t reason)
          : consumed (consumed)
          , state (s)
          , reason (reason)
        {}

        size_t       consumed;
        state_t      state;
        error_code_t reason;
      };

      class parser
      {
      public:
        parser ();

        void reset ();

        result_t parse ( const char* begin
                       , const char* end
                       , gspc::net::frame & frame
                       );
      private:
        enum frame_state
          {
            frame_start
          , frame_start_line_feed
          , command
          , command_line_feed
          , header_start
          , header_start_line_feed
          , header_name
          , header_value
          , header_value_line_feed
          , body_without_content_length
          , body_with_content_length
          , body_with_content_length_final_null
          };

        state_t consume (gspc::net::frame & frame, const char c);

        frame_state                  m_frame_state;
        error_code_t                 m_error;
        std::string                  m_buffer;
        std::string                  m_header_key;
        std::size_t                  m_remaining_body_bytes;
      };
    }
  }
}

#endif
