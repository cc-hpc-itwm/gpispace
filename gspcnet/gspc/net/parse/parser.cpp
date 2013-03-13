#include "parser.hpp"

namespace gspc
{
  namespace net
  {
    namespace parse
    {
      parser::parser ()
        : m_frame_state (command_start)
      {}

      void parser::reset ()
      {
        m_frame_state = command_start;
        m_buffer.clear ();
      }

      state_t parser::consume ( gspc::net::frame & frame
                              , const char c
                              )
      {
        return gspc::net::parse::PARSE_FAILED;
      }
    }
  }
}
