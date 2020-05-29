#pragma once

#include <we/type/bytearray.hpp>

#include <boost/filesystem.hpp>

#include <string>

namespace transform_file
{
  class parameter
  {
  public:
    parameter ( boost::filesystem::path const& input
              , boost::filesystem::path const& output
              , unsigned long size
              )
      : _input (input)
      , _output (output)
      , _size (size)
    {}

    boost::filesystem::path const& input() const
    {
      return _input;
    }
    boost::filesystem::path const& output() const
    {
      return _output;
    }
    unsigned long size() const
    {
      return _size;
    }

  private:
    boost::filesystem::path const _input;
    boost::filesystem::path const _output;
    unsigned long _size;
  };

  parameter from_bytearray (we::type::bytearray const&);
  we::type::bytearray to_bytearray (parameter const&);
}
