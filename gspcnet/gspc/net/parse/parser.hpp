#ifndef GSPC_NET_PARSER_HPP
#define GSPC_NET_PARSER_HPP

#include <string>

namespace gspc
{
  namespace net
  {
    class frame;

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
        {}

        result_t (size_t consumed, state_t s)
          : consumed (consumed)
          , state (s)
        {}

        size_t      consumed;
        state_t     state;
      };

      class parser
      {
      public:
        parser ();

        void reset ();

        template <typename InputIterator>
        result_t parse ( InputIterator begin
                       , InputIterator end
                       , gspc::net::frame & frame
                       )
        {
          size_t consumed = 0;

          while (begin != end)
          {
            state_t state = consume (frame, *begin++);
            ++consumed;

            if (state == PARSE_FINISHED || state == PARSE_FAILED)
              return result_t (consumed, state);
          }

          return result_t ( consumed
                          , PARSE_NEED_MORE_DATA
                          );
        }
      private:
        enum frame_state
          {
            command_start
          , command
          , header_start
          , header_name
          , header_value
          , body_start
          , body
          , body_end
          } m_state;

        state_t consume (gspc::net::frame & frame, const char c);

        frame_state       m_frame_state;
        std::string       m_buffer;
      };
    }
  }
}

#endif
