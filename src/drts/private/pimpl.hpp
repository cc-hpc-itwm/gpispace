#pragma once

#define PIMPL_IMPLEMENTATION(_name, _type...) \
  struct _name::implementation                \
  {                                           \
    implementation (_type const& _name)       \
      : _ (_name)                             \
    {}                                        \
                                              \
    _type const _;                            \
  };                                          \
                                              \
  PIMPL_DTOR (_name)

#define PIMPL_DTOR(_name)                     \
  _name::~_name()                             \
  {                                           \
    delete _;                                 \
    _ = nullptr;                              \
  }
