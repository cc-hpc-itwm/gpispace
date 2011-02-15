#ifndef GPI_SPACE_PC_CMDLINE_COMMAND_HPP
#define GPI_SPACE_PC_CMDLINE_COMMAND_HPP 1

#include <string>

namespace gpi
{
  namespace pc
  {
    namespace cmdline
    {
      class command_t
      {
      public:
        command_t (const std::string & name, const std::string & doc);

        virtual ~command_t () {}

        virtual void operator () (const std::string & args) = 0;

        std::string const & name () const { return m_name; }
        std::string const & doc  () const { return m_doc; }
      private:
        std::string m_name;
        std::string m_doc;
      };
    }
  }
}

#endif
