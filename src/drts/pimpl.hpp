// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#define PIMPL( _name)                 \
  public:                             \
    ~_name();                         \
  private:                            \
    struct implementation;            \
    std::unique_ptr<implementation> _
