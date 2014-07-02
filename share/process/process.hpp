#ifndef APPLICATION_EXEC_HPP
#define APPLICATION_EXEC_HPP 1

#include <string>
#include <iostream>
#include <list>
#include <vector>
#include <map>

namespace process
{
  namespace detail
  {
    template<typename T> struct buffer
    {
    private:
      T _buf;
      const std::size_t _size;

    public:
      buffer (T buf, const std::size_t& size)
        : _buf (buf)
        , _size (size)
      {}

      T buf() const { return _buf; }
      const std::size_t& size() const { return _size; }
    };

    template<typename T> struct file_buffer
    {
    private:
      typedef buffer<T> buffer_t;

      const buffer_t _buf;
      const std::string _param;

    public:
      file_buffer (const buffer_t& buf, const std::string& param)
        : _buf (buf)
        , _param (param)
      {}

      file_buffer (T buf, const std::size_t& size, const std::string& param)
        : _buf (buf, size)
        , _param (param)
      {}

      buffer_t const& to_buffer() const { return _buf; }
      T buf() const { return _buf.buf(); }
      const std::size_t& size() const { return _buf.size(); }
      const std::string& param() const { return _param; }
    };
  }

  typedef detail::buffer<const void*> const_buffer;
  typedef detail::buffer<void*> buffer;
  typedef std::vector<char> circular_buffer;

  typedef detail::file_buffer<const void*> file_const_buffer;
  typedef detail::file_buffer<void*> file_buffer;

  typedef std::list<file_const_buffer> file_const_buffer_list;
  typedef std::list<file_buffer> file_buffer_list;

  struct execute_return_type
  {
  public:
    typedef std::size_t size_type;
    typedef std::vector<size_type> size_list_type;

    size_type bytes_written_stdin;
    size_type bytes_read_stdout;
    size_type bytes_read_stderr;
    size_list_type bytes_read_files_output;
    int exit_code;

    execute_return_type (size_type files_output)
      : bytes_written_stdin (0)
      , bytes_read_stdout (0)
      , bytes_read_stderr (0)
      , bytes_read_files_output (files_output)
      , exit_code (255)
    {}
  };

  // ********************************************************************** //

  extern execute_return_type execute ( std::string const& command
                                     , const_buffer const& buf_stdin
                                     , buffer const& buf_stdout
                                     , circular_buffer& buf_stderr
                                     , file_const_buffer_list const& files_input
                                     , file_buffer_list const& files_output
                                     , std::map<std::string, std::string> const &environment
                                     );

  extern execute_return_type execute ( std::string const& command
                                     , const_buffer const& buf_stdin
                                     , buffer const& buf_stdout
                                     , circular_buffer& buf_stderr
                                     , file_const_buffer_list const& files_input
                                     , file_buffer_list const& files_output
                                     );

  inline execute_return_type execute ( std::string const& command
                                     , const_buffer const&  buf_stdin
                                     , buffer const& buf_stdout
                                     , file_const_buffer_list const& files_input
                                     , file_buffer_list const& files_output
                                     )
  {
    circular_buffer buf_stderr;

    return execute
      (command, buf_stdin, buf_stdout, buf_stderr, files_input, files_output);
  }

  inline std::size_t execute ( std::string const& command
                             , const void* input
                             , const std::size_t& input_size
                             , void* output
                             , const std::size_t& max_output_size
                             )
  {
    return execute ( command
                   , const_buffer (input, input_size)
                   , buffer (output, max_output_size)
                   , file_const_buffer_list()
                   , file_buffer_list()
                   ).bytes_read_stdout;
  }
}

#endif
