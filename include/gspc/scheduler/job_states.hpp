// Copyright (C) 2012-2016,2018,2023-2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <stdexcept>


  namespace gspc::scheduler::status
  {
    enum code
      {
        // terminal states
        FINISHED
      , FAILED
      , CANCELED

        // non-terminal states
      , PENDING
      , RUNNING
      , CANCELING
      };

    inline bool is_terminal (code c)
    {
      return c == FINISHED || c == FAILED || c == CANCELED;
    }

    inline std::string show (status::code code)
    {
      switch (code)
      {
      case PENDING:
        return "Scheduler::Pending";
      case RUNNING:
        return "Scheduler::Running";
      case FINISHED:
        return "Scheduler::Finished";
      case FAILED:
        return "Scheduler::Failed";
      case CANCELED:
        return "Scheduler::Canceled";
      case CANCELING:
        return "Scheduler::Canceling";
      }

      throw std::logic_error {"invalid enum value"};
    }
  }
