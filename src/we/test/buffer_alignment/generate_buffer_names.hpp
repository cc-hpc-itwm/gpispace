#pragma once

#include <util-generic/testing/random.hpp>

#include <algorithm>
#include <list>
#include <string>

namespace
{
  std::string get_new_buffer_name (std::list<std::string> const& buffers)
  {
    std::string buffer_name;

    do
    {
      buffer_name = fhg::util::testing::random_char_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
      buffer_name += fhg::util::testing::random_string_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
    } while (std::find (buffers.begin(), buffers.end(), buffer_name) != buffers.end());

    return buffer_name;
  }
}
