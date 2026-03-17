// Copyright (C) 2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/util/signal_handler_Execution_can_be_stopped.hpp>

#include <gspc/util/syscall.hpp>

TEST (gspc::util::syscall::kill (gspc::util::syscall::getpid(), SIGINT))
