// Copyright (C) 2014,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <map/transform_file/type.hpp>

namespace transform_file
{
  namespace
  {
    enum field {INPUT, OUTPUT, SIZE, NUM_FIELDS};
  }

  parameter from_bytearray (gspc::we::type::bytearray const& ba)
  {
    std::string const s (ba.to_string());
    unsigned long const size_header (NUM_FIELDS * sizeof (unsigned long));

    if (s.size() < size_header)
    {
      throw std::runtime_error ("Bytearray shorter than header.");
    }

    std::vector<char> data (s.begin(), s.begin() + size_header);

    auto* sizes {reinterpret_cast<unsigned long*> (data.data())};

    unsigned long const size_input {sizes[INPUT]};
    unsigned long const size_output {sizes[OUTPUT]};
    unsigned long const size {sizes[SIZE]};

    if (s.size() < size_header + size_input + size_output)
    {
      throw std::runtime_error ("Bytearray to short, inconsistent header");
    }

    return parameter
      ( std::filesystem::path (s.substr (size_header, size_input))
      , std::filesystem::path (s.substr (size_header + size_input, size_output))
      , size
      );
  }

  gspc::we::type::bytearray to_bytearray (parameter const& p)
  {
    auto const input {p.input().string()};
    auto const output {p.output().string()};
    unsigned long const size_header {NUM_FIELDS * sizeof (unsigned long)};
    unsigned long const size_input {input.size()};
    unsigned long const size_output {output.size()};

    std::vector<char> data (size_header + size_input + size_output);

    auto* sizes {reinterpret_cast<unsigned long*> (data.data())};

    sizes[INPUT] = size_input;
    sizes[OUTPUT] = size_output;
    sizes[SIZE] = p.size();

    std::copy ( input.begin(), input.end()
              , data.data() + size_header
              );
    std::copy ( output.begin(), output.end()
              , data.data() + size_header + size_input
              );

    return gspc::we::type::bytearray
      (data.data(), size_header + size_input + size_output);
  }
}
