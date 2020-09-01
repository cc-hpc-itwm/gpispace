#pragma once

#include <memory>

#define PIMPL( _name)                 \
  public:                             \
    ~_name();                         \
  private:                            \
    struct implementation;            \
    std::unique_ptr<implementation> _
