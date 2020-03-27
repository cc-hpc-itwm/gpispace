#pragma once

#include <list>
#include <string>

std::string create_buffer_description
  ( std::string const& name
  , std::size_t size
  );

std::string create_buffer_description
  ( std::string const& name
  , std::size_t size
  , std::size_t alignment
  );

std::string create_alignment_test
  ( std::size_t alignment
  , std::string const& name
  );

std::string create_net_description
  ( std::string const& buffer_descriptions
  , std::list<std::string> const& buffer_names
  , std::string const& alignment_tests
  );
