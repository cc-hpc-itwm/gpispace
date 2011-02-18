#ifndef GPI_SPACE_SHELL_SHELL_HPP
#define GPI_SPACE_SHELL_SHELL_HPP 1

#include <string>
#include <vector>

#include <boost/function.hpp>

#include <gpi-space/shell/command.hpp>

namespace gpi
{
  namespace shell
  {
    template <typename State>
    class basic_shell_t
    {
    public:
      typedef State state_type;
      typedef basic_shell_t<State> self;
      typedef std::vector <std::string> argv_t;

      typedef boost::function<int (argv_t const &, self &)> command_callback_t;
      typedef basic_command_t<self> command_t;
      typedef std::vector <command_t> command_list_t;

      static self & create ( std::string const & program_name
                           , std::string const & prompt
                           , state_type & state
                           );
      static self & get ();
      static void destroy ();

      void add_command ( std::string const & name
                       , command_callback_t callback
                       , std::string const & short_doc
                       );

      void add_command ( std::string const & name
                       , command_callback_t callback
                       , std::string const & short_doc
                       , std::string const & long_doc
                       );

      int execute (std::string const & line);
      int execute ( argv_t const & argv );
      const command_t * find_command (std::string const & name) const;

      command_list_t const & commands () const { return m_commands; }
      state_type & state () { return m_state; }

      int run ();
      int run_once ();
      void reset ();
      void stop ();

    private:
      static self* instance;

      static void initialize_readline ();

      static char **shell_completion (const char *, int, int);
      static char *command_generator (const char *, int);

      basic_shell_t ( std::string const & program_name
                    , std::string const & prompt
                    , state_type & state
                    );

      std::string m_program_name;
      std::string m_prompt;
      state_type &m_state;
      command_list_t m_commands;
      bool m_do_exit;
    };
  }
}

#include "shell.ipp"

#endif
