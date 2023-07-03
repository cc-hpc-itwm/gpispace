// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace fhg
{
  namespace util
  {
    int wexitstatus (int);
    int wifcontinued (int);
    int wifexited (int);
    int wifsignaled (int);
    int wifstopped (int);

    int wstopsig (int);
    int wtermsig (int);
  }
}
