#pragma once

#define PIMPL( _name)             \
  public:                         \
    ~_name();                     \
  private:                        \
    struct implementation;        \
    implementation* _
