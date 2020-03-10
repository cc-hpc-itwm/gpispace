#pragma once

#include <util-generic/testing/random.hpp>

#include <string>

namespace
{
  std::string get_new_buffer_name (std::string const& buffer_names)
  {
    std::string buffer_name;

    do
    {
      buffer_name = fhg::util::testing::random_char_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
      buffer_name += fhg::util::testing::random_string_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
    } while (buffer_names.find (buffer_name) != std::string::npos);

    return buffer_name;
  }
}
