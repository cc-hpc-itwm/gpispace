// Copyright (C) 2019,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later


  namespace gspc::rif
  {
    template<typename... String>
      void started_process_promise::set_result (String... messages)
    {
      return set_result (std::vector<std::string> {messages...});
    }
  }
