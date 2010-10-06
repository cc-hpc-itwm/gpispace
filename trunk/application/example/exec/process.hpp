#ifndef APPLICATION_EXEC_HPP
#define APPLICATION_EXEC_HPP 1

#include <string>
#include <unistd.h>

namespace process
{
  class process_t
  {
  public:
    explicit
    process_t (std::string const & commandline);

    void start();
    void communicate ( const void * input, const std::size_t input_size
                     , void * output, const std::size_t output_size
                     , const std::size_t buffer_size
                     );
    int wait (int options = 0);
    int terminate ();
    bool is_alive () const;
  private:
    void child (const pid_t parent_pid);
    int initialize_pipes();
    std::string command_line_;
    mutable pid_t child_pid_;
    mutable int exit_code_;
    int pipe_[2][3];
  };

  extern int execute ( std::string const & commandline
                     , const void * input, const std::size_t input_size
                     , void * output, const std::size_t output_size
                     , const std::size_t buffer_size
                     );
}

#endif
