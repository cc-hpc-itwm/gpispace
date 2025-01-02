// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

// \note Not using c++11 attribute syntax `[[gnu::visibility
// ("default")]]`, because we mark friend functions as exported as
// well, inside the class definition. That is supported with the old
// syntax, but not the new one, where it requires a friend with
// attribute to be a type (although clang disagrees and doesn't even
// allow attributes for types).
// \todo Compiler detection if we ever bother about non gcc-compatible?
#define IML_DLLEXPORT_IMPL __attribute__ ((visibility ("default")))
