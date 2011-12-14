#ifndef GPI_SPACE_SHELL_COMMAND_HPP
#define GPI_SPACE_SHELL_COMMAND_HPP 1

#include <string>

namespace gpi
{
  namespace shell
  {
    namespace util
    {
      inline std::string stripws (const std::string &);
    }

    template <typename Shell>
    class basic_command_t
    {
    public:
      typedef Shell shell_t;
      typedef typename shell_t::argv_t argv_t;
      typedef typename shell_t::command_callback_t callback_t;

      basic_command_t ( const std::string & nme
                      , const std::string & short_doc
                      , const std::string & long_doc
                      , callback_t cb
                      )
        : m_name (nme)
        , m_short_doc (short_doc)
        , m_long_doc (long_doc)
        , m_callback (cb)
      {}

      virtual ~basic_command_t () {}

      int operator () (argv_t const & args, shell_t &shell) const
      {
        return m_callback (args, shell);
      }

      std::string const & name () const { return m_name; }
      std::string const & short_doc  () const { return m_short_doc; }
      std::string const & long_doc  () const { return m_long_doc; }
    private:
      std::string m_name;
      std::string m_short_doc;
      std::string m_long_doc;
      callback_t m_callback;
    };
  }
}

#endif
