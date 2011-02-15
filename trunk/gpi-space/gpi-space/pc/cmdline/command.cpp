#include "command.hpp"

namespace gpi
{
  namespace pc
  {
    namespace cmdline
    {
      command_t::command_t (const std::string & name, const std::string & doc)
        : m_name (name)
        , m_doc (doc)
      {}
    }
  }
}
