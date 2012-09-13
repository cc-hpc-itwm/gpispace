/* -*- mode: c++ -*- */

#include "shell.hpp"

// readline
#include <unistd.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <boost/algorithm/string.hpp>
#include <fhg/assert.hpp>

namespace gpi
{
  namespace shell
  {
    namespace detail
    {
      inline
      bool readline (const std::string & prompt, std::string & line_read)
      {
        char * line;
        line = ::readline (prompt.c_str());
        if (!line)
        {
          return false;
        }

        line_read = line;
        free (line);

        boost::algorithm::trim_left (line_read);
        if (line_read.size())
        {
          add_history (line_read.c_str());
        }

        return true;
      }

      template <typename Vec>
      void split (Vec & v, std::string const & s)
      {
        boost::algorithm::split (v, s, boost::algorithm::is_space(), boost::algorithm::token_compress_on);
      }
    }

    template <typename S>
    basic_shell_t<S>* basic_shell_t<S>::instance = NULL;

    template <typename S>
    basic_shell_t<S> & basic_shell_t<S>::create ( std::string const & program_name
                                                , std::string const & prompt
                                                , std::string const & histfile
                                                , state_type & state
                                                )
    {
      if (instance == NULL)
      {
        instance = new basic_shell_t<S> (program_name, prompt, histfile, state);
      }
      return *instance;
    }

    template <typename S>
    basic_shell_t<S> & basic_shell_t<S>::get ()
    {
      assert (instance != NULL);
      return *instance;
    }

    template <typename S>
    void basic_shell_t<S>::destroy ()
    {
      if (instance)
      {
        delete instance;
      }
      instance = NULL;
    }

    template <typename S>
    void basic_shell_t<S>::initialize_readline ()
    {
      rl_readline_name = m_program_name.c_str();
      // set completer
      rl_attempted_completion_function = &self::shell_completion;
      using_history();
      read_history();
    }

    template <typename S>
    void basic_shell_t<S>::finalize_readline ()
    {
      write_history();
    }

    template <typename S>
    void basic_shell_t<S>::read_history()
    {
      if (! m_histfile.empty())
      {
        ::read_history (m_histfile.c_str());
      }
    }

    template <typename S>
    void basic_shell_t<S>::write_history()
    {
      if (! m_histfile.empty())
      {
        ::write_history (m_histfile.c_str());
      }
    }

    template <typename S>
    basic_shell_t<S>::~basic_shell_t ()
    {
      finalize_readline();
    }

    template <typename S>
    basic_shell_t<S>::basic_shell_t ( std::string const & program_name
                                    , std::string const & prompt
                                    , std::string const & histfile
                                    , state_type & state
                                    )
      : m_program_name (program_name)
      , m_prompt (prompt)
      , m_histfile (histfile)
      , m_state (state)
      , m_do_exit (false)
    {
      initialize_readline ();
    }

    template <typename S>
    void basic_shell_t<S>::add_command ( std::string const & name
                                       , typename basic_shell_t<S>::command_callback_t callback
                                       , std::string const & short_doc
                                       )
    {
      add_command (name, callback, short_doc, short_doc);
    }

    template <typename S>
    void basic_shell_t<S>::add_command ( std::string const & name
                                       , typename basic_shell_t<S>::command_callback_t callback
                                       , std::string const & short_doc
                                       , std::string const & long_doc
                                       )
    {
      m_commands.push_back (command_t(name, short_doc, long_doc, callback));
    }

    template <typename S>
    const typename basic_shell_t<S>::command_t *
    basic_shell_t<S>::find_command ( std::string const & name ) const
    {
      for ( typename command_list_t::const_iterator cmd (m_commands.begin())
          ; cmd != m_commands.end()
          ; ++cmd
          )
      {
        if (cmd->name() == name)
        {
          return &(*cmd);
        }
      }
      return (command_t*)(NULL);
    }

    template <typename S>
    int basic_shell_t<S>::run_once ()
    {
      std::string line;
      int rc;

      rc = 0;

      if (detail::readline (m_prompt.c_str(), line))
      {
        boost::algorithm::trim (line);

        if (line.empty())
          return rc;

        argv_t argv;
        detail::split (argv, line);
        rc = execute (argv);
        if (-1 == rc)
        {
          rc = m_state.error (std::runtime_error ("command not found: " + argv[0]));
          if (rc != 0)
          {
            stop ();
          }
        }
      }
      else
      {
        stop ();
      }

      return rc;
    }

    template <typename S>
    void basic_shell_t<S>::reset ()
    {
      m_do_exit = false;
    }

    template <typename S>
    void basic_shell_t<S>::stop ()
    {
      m_do_exit = true;
    }

    template <typename S>
    int basic_shell_t<S>::run ()
    {
      int rc (0);
      while (!m_do_exit)
      {
        rc = run_once ();
      }
      return rc;
    }

    template <typename S>
    int basic_shell_t<S>::execute (std::string const & line)
    {
      argv_t av;
      detail::split (av, line);
      return execute (av);
    }

    template <typename S>
    int basic_shell_t<S>::execute (argv_t const & argv)
    {
      int rc(0);

      if (argv.empty())
      {
        std::cerr << "empty command!" << std::endl;
        return -1;
      }

      // 2. call command
      const command_t *cmd (find_command(argv[0]));
      if (cmd)
      {
        try
        {
          rc = (*cmd)(argv, *this);
        }
        catch (std::exception const & ex)
        {
          std::cerr << "failed: " << ex.what() << std::endl;
          rc = -2;
        }
      }
      else
      {
        rc = -1;
      }

      return rc;
    }

    /* interface to readline completion      */
    /* does only work with a shell singleton */

    template <typename S>
    char * basic_shell_t<S>::command_generator (const char *text, int state)
    {
      static std::size_t command_index (0), text_length (0);

      if (!state)
      {
        // initialize state variables
        command_index = 0;
        text_length = strlen (text);
      }

      // check commands for a match
      const typename self::command_list_t & commands (self::get ().commands());
      while (command_index < commands.size())
      {
        const char *cmd_name = commands[command_index].name().c_str();
        ++command_index;
        if (strncmp(cmd_name, text, text_length) == 0)
        {
          char *match = (char*)malloc(strlen(cmd_name) + 1);
          strcpy (match, cmd_name);
          return match;
        }
      }
      return NULL;
    }

    template <typename S>
    char **basic_shell_t<S>::shell_completion (const char * text, int start, int end)
    {
      char **matches (NULL);

      if (start == 0)
      {
        matches = rl_completion_matches (text, &self::command_generator);
      }

      return matches;
    }
  }
}
