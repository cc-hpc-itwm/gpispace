// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifndef NDEBUG
#define IFNDEF_NDEBUG(x...) x
#define IFDEF_NDEBUG(x...)
#else
#define IFNDEF_NDEBUG(x...)
#define IFDEF_NDEBUG(x...) x
#endif
